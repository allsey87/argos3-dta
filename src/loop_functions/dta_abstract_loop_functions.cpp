#include "dta_abstract_loop_functions.h"

#include <argos3/core/simulator/entity/floor_entity.h>
#include <argos3/core/utility/math/rng.h>
#include <argos3/plugins/simulator/entities/debug_entity.h>
#include <argos3/plugins/simulator/media/radio_medium.h>
#include <argos3/plugins/robots/generic/simulator/wifi_default_actuator.h>
#include <argos3/plugins/robots/pi-puck/simulator/pipuck_entity.h>

#define CSV_HEADER "\"foraging robots\"\t\"building robots\"\t\"average estimate\"\t\"average deviation\"\t\"construction events\"\t\"blocks in cache\""

namespace argos {

   class CDTAAbstractWifiActuator : public CWifiDefaultActuator {
   public:
      class CTxOperation : public CPositionalIndex<CRadioEntity>::COperation {
      public:
         CTxOperation(const CRadioEntity& c_tx_radio,
                      const std::list<CByteArray>& lst_messages,
                      const std::set<std::string>& set_can_send_to) :
            m_cTxRadio(c_tx_radio),
            m_lstMessages(lst_messages),
            m_setCanSendTo(set_can_send_to) {}

         virtual bool operator()(CRadioEntity& c_rx_radio) {
            if(m_setCanSendTo.count(c_rx_radio.GetRootEntity().GetId()) != 0) {
               const CVector3& cTxRadioPosition = m_cTxRadio.GetPosition();
               for(const CByteArray& c_data : m_lstMessages) {
                  c_rx_radio.ReceiveData(cTxRadioPosition, c_data);
               }
            }
            return true;
         }

      private:
         const CRadioEntity& m_cTxRadio;
         const std::list<CByteArray>& m_lstMessages;
         const std::set<std::string>& m_setCanSendTo;
      };

      /****************************************/
      /****************************************/

      virtual void Update() override {
         if(!m_lstMessages.empty()) {
            CTxOperation cTxOperation(*m_pcRadioEntity,
                                      m_lstMessages,
                                      m_setCanSendTo);
            m_pcRadioEntity->GetMedium().GetIndex().ForAllEntities(cTxOperation);
            m_lstMessages.clear();
         }
      }

      /****************************************/
      /****************************************/

      void SetCanSendTo(const std::set<std::string>& set_can_send_to) {
         m_setCanSendTo = set_can_send_to;
      }

   private:
      std::set<std::string> m_setCanSendTo;
   };

   /****************************************/
   /****************************************/

   REGISTER_ACTUATOR(CDTAAbstractWifiActuator,
                     "wifi", "dta_abstract",
                     "Michael Allwright [allsey87@gmail.com]",
                     "1.0",
                     "An abstract wifi actuator for the DTA experiments.",
                     "This actuator sends messages over wifi.",
                     "Usable"
   );

   /****************************************/
   /****************************************/

   CDTAAbstractLoopFunctions::CDTAAbstractLoopFunctions() :
      m_unStepsUntilShadeCell(0) {
      /* calculate the ticks per second */
      UInt32 unTicksPerSecond =
         static_cast<UInt32>(std::round(CPhysicsEngine::GetInverseSimulationClockTick()));
      m_vecConstructionEvents.assign(unTicksPerSecond, 0);
   }

   /****************************************/
   /****************************************/

   void CDTAAbstractLoopFunctions::Init(TConfigurationNode& t_tree) {
	   //LOG << "Initializing loop functions ..." << std::endl;
      GetNodeAttributeOrDefault(t_tree, "output", m_strOutputFilename, m_strOutputFilename);
      if(!m_strOutputFilename.empty()) {
         m_cOutputFile.open(m_strOutputFilename, std::ios_base::out | std::ios_base::trunc);
         if(m_cOutputFile.is_open() && m_cOutputFile.good()) {
            m_cOutputFile << CSV_HEADER << std::endl;
         }
      }
      /* parse the parameters */
      TConfigurationNode& tParameters = GetNode(t_tree, "parameters");
      /* parse the foraging delay coefficient and the construction limit */
      GetNodeAttribute(tParameters, "foraging_delay_coeff", m_fForagingDelayCoefficient);
      GetNodeAttribute(tParameters, "foraging_duration_mean", m_fForagingDurationMean);
      GetNodeAttribute(tParameters, "construction_limit", m_unConstructionLimit);
      bool b_complet_network = false;
      GetNodeAttribute(tParameters, "complete_network", b_complet_network);
      /* parse the grid configuration */
      std::string strGridLayout;
      GetNodeAttribute(tParameters, "grid_layout", strGridLayout);
      ParseValues<UInt32>(strGridLayout, 2, m_arrGridLayout.data(), ',');
      /* initialize vector of cells */
      m_vecCells.assign(m_arrGridLayout[0] * m_arrGridLayout[1], false);
      GetSpace().GetFloorEntity().SetChanged();
      /* create a map of the pi-puck robots */
      TConfigurationNodeIterator itPiPuck("pipuck");
      std::string strId;
      std::string strController;
      std::string strCanSendTo;
      std::vector<std::string> vecCanSendTo;
      std::set<std::string> setCanSendTo;
      
      if(b_complet_network){
		  // --- THIS IMPLEMENTS A COMPLETE NETWORK ---
		  for(itPiPuck = itPiPuck.begin(&GetNode(t_tree,"robots"));
			  itPiPuck != itPiPuck.end();
			  ++itPiPuck) {
			 GetNodeAttribute(*itPiPuck, "id", strId);
			 vecCanSendTo.push_back(strId);
		  }
		  for(itPiPuck = itPiPuck.begin(&GetNode(t_tree,"robots"));
			  itPiPuck != itPiPuck.end();
			  ++itPiPuck) {
			 GetNodeAttribute(*itPiPuck, "id", strId);
			 GetNodeAttribute(*itPiPuck, "controller", strController);
			 GetNodeAttribute(*itPiPuck, "can_send_to", strCanSendTo);
			 setCanSendTo.clear();
			 setCanSendTo.insert(std::begin(vecCanSendTo),
								 std::end(vecCanSendTo));
			 m_mapRobots.emplace(std::piecewise_construct,
								 std::forward_as_tuple(strId),
								 std::forward_as_tuple(strController, setCanSendTo));
		  }
		  // --------------------------------------------
	  }
	  else{
		  // --- THIS IMPLEMENTS A NETWORK SET IN THE ARGOS FILE ---
		  for(itPiPuck = itPiPuck.begin(&GetNode(t_tree,"robots"));
			  itPiPuck != itPiPuck.end();
			  ++itPiPuck) {
			 GetNodeAttribute(*itPiPuck, "id", strId);
			 GetNodeAttribute(*itPiPuck, "controller", strController);
			 GetNodeAttribute(*itPiPuck, "can_send_to", strCanSendTo);
			 vecCanSendTo.clear();
			 setCanSendTo.clear();
			 Tokenize(strCanSendTo, vecCanSendTo, ",");
			 setCanSendTo.insert(std::begin(vecCanSendTo),
								 std::end(vecCanSendTo));
			 m_mapRobots.emplace(std::piecewise_construct,
								 std::forward_as_tuple(strId),
								 std::forward_as_tuple(strController, setCanSendTo));
		  }
		  //~ // -------------------------------------------------------
	  }
      
		m_pcRNG = CRandom::CreateRNG("argos");
    
    for(size_t i = 0; i < m_vecCells.size(); i++) {
		m_vecCells[i] = (m_pcRNG->Uniform(CRange<Real>(0.0, 1.0)) > 1.0);
	}  
   }

   /****************************************/
   /****************************************/

   void CDTAAbstractLoopFunctions::Reset() {
      /* remove all robots */
      for(std::pair<const std::string, SPiPuck>& c_pair : m_mapRobots) {
         SPiPuck& s_pipuck = c_pair.second;
         RemoveEntity(*s_pipuck.Entity);
         s_pipuck.Entity = nullptr;
         s_pipuck.StepsUntilReturnToConstructionTask = 0;
      }
      /* clear all cells and mark the floor as changed */
      m_vecCells.assign(m_vecCells.size(), false);
    
    for(size_t i = 0; i < m_vecCells.size(); i++) {
		m_vecCells[i] = (m_pcRNG->Uniform(CRange<Real>(0.0, 1.0)) > 0.5);
	}   
      GetSpace().GetFloorEntity().SetChanged();
      /* zero out the construction events */
      m_vecConstructionEvents.assign(m_vecConstructionEvents.size(), 0);
      if(m_cOutputFile.is_open()) {
         m_cOutputFile.close();
         m_cOutputFile.clear();
      }
      if(!m_strOutputFilename.empty()) {
         m_cOutputFile.open(m_strOutputFilename, std::ios_base::out | std::ios_base::trunc);
         if(m_cOutputFile.is_open() && m_cOutputFile.good()) {
            m_cOutputFile << CSV_HEADER << std::endl;
         }
      }
   }

   /****************************************/
   /****************************************/

   void CDTAAbstractLoopFunctions::Destroy() {
      if(m_cOutputFile.is_open()) {
         m_cOutputFile.close();
      }
   }

   /****************************************/
   /****************************************/

   CColor CDTAAbstractLoopFunctions::GetFloorColor(const CVector2& c_position) {
      const CVector3& cArenaSize = GetSpace().GetArenaSize();
      /* convert c_position into cell coordinates */
      UInt32 unX = static_cast<UInt32>(c_position.GetX() * m_arrGridLayout.at(0) / cArenaSize.GetX());
      UInt32 unY = static_cast<UInt32>(c_position.GetY() * m_arrGridLayout.at(1) / cArenaSize.GetY());
      /* shade cell if occupied */
      return m_vecCells.at(unX + m_arrGridLayout[0] * unY) ? CColor::GRAY50 : CColor::GRAY80;
   }

   /****************************************/
   /****************************************/

   void CDTAAbstractLoopFunctions::PostStep() {
	   int time = GetSpace().GetSimulationClock();
      /* total number of robots */
      UInt32 unRobotsCount = m_mapRobots.size();
      /* count the robots that are foraging */
      UInt32 unForagingRobotsCount =
         std::count_if(std::begin(m_mapRobots),
                       std::end(m_mapRobots),
                       [] (const std::pair<const std::string, SPiPuck>& s_pipuck) {
            return (s_pipuck.second.Entity == nullptr);
         }
      );
      /* calculate the robots that are building */
      UInt32 unBuildingRobotsCount = unRobotsCount - unForagingRobotsCount;
      UInt32 unEstimatingRobotsCount = 0;
      /* calculate the number of shaded cells */
      UInt32 unShadedCells =
            std::count(std::begin(m_vecCells), std::end(m_vecCells), true);
      /* get the current estimate value from each building robot */
	  Real fEstimate = 0.0, avg_fEstimate = 0.0, fDeviation = 0.0, avg_fDeviation = 0.0;
	  std::string strTask;
      if(unBuildingRobotsCount > 0){
		  for(std::pair<const std::string, SPiPuck>& c_pair : m_mapRobots) {
			 SPiPuck& sPiPuck = c_pair.second;
			 //const std::string& strId = c_pair.first;
			 /* only consider robots currently performing the construction task */
			 if(sPiPuck.Entity != nullptr) {
				const std::string& strSetTaskBuffer =
				   sPiPuck.Entity->GetDebugEntity().GetBuffer("set_task");
				std::stringstream cTask(strSetTaskBuffer);
				cTask >> strTask;
				
				if(strTask=="exploring"){
					const std::string& strEstimateBuffer =
					   sPiPuck.Entity->GetDebugEntity().GetBuffer("set_estimate");
					std::stringstream cEstimate(strEstimateBuffer);
					cEstimate >> fEstimate;
					avg_fEstimate += fEstimate;
					
					const std::string& strDeviationBuffer =
					   sPiPuck.Entity->GetDebugEntity().GetBuffer("set_gradient");
					std::stringstream cDeviation(strDeviationBuffer);
					cDeviation >> fDeviation;
					avg_fDeviation += fDeviation;
					unEstimatingRobotsCount++;
				}
			 }
		  }
	  }
      
	  UInt32 unTotalConstructionEvents = 0;
	  for(UInt32 un_count : m_vecConstructionEvents) {
		 unTotalConstructionEvents += un_count;
	  }
      // --- print the results ---
      if(unEstimatingRobotsCount > 0){
		  avg_fEstimate = avg_fEstimate/(1.0*unEstimatingRobotsCount);
		  avg_fDeviation = avg_fDeviation/(1.0*unEstimatingRobotsCount);
		  LOGERR << unForagingRobotsCount << "\t"
						   << unBuildingRobotsCount << "\t"
						   << avg_fEstimate << "\t"
						   << avg_fDeviation << "\t"
						   << unTotalConstructionEvents << "\t"
						   << unShadedCells/(1.0*625) << std::endl;
		  /* write output */
		  if(m_cOutputFile.is_open() && m_cOutputFile.good()) {
			 m_cOutputFile << unForagingRobotsCount << "\t"
						   << unBuildingRobotsCount << "\t"
						   << avg_fEstimate << "\t"
						   << avg_fDeviation << "\t"
						   << unTotalConstructionEvents << "\t"
						   << unShadedCells/(1.0*625) << std::endl;
		  }
	  }
	  
      /* handle cell unshading */
      /* get the number of block attachments for this step */
      UInt32& unConstructionEvents =
         m_vecConstructionEvents.at(GetSpace().GetSimulationClock() % m_vecConstructionEvents.size());
      /* reset the current value to zero */
      unConstructionEvents = 0;
      const CVector3& cArenaSize = GetSpace().GetArenaSize();
      for(std::pair<const std::string, SPiPuck>& c_pair : m_mapRobots) {
         SPiPuck& sPiPuck = c_pair.second;
         if(sPiPuck.Entity != nullptr) {
            const CVector3& cPosition =
               sPiPuck.Entity->GetEmbodiedEntity().GetOriginAnchor().Position;
            UInt32 unX = static_cast<UInt32>(cPosition.GetX() * m_arrGridLayout.at(0) / cArenaSize.GetX());
            UInt32 unY = static_cast<UInt32>(cPosition.GetY() * m_arrGridLayout.at(1) / cArenaSize.GetY());
            /* check if robot has moved to a different cell */
            if((unX != sPiPuck.PreviousX) || (unY != sPiPuck.PreviousY)) {
               UInt32 unCellIndex = sPiPuck.PreviousX + m_arrGridLayout[0] * sPiPuck.PreviousY;
               if(m_vecCells.at(unCellIndex)) {
                  UInt32 unTotalConstructionEvents = 0;
                  for(UInt32 un_count : m_vecConstructionEvents) {
                     unTotalConstructionEvents += un_count;
                  }
                  if(unTotalConstructionEvents < m_unConstructionLimit) {
                     unConstructionEvents += 1;
                     m_vecCells[unCellIndex] = false;
                  GetSpace().GetFloorEntity().SetChanged();
                  }
               }
               sPiPuck.PreviousX = unX;
               sPiPuck.PreviousY = unY;
			 }
		  }
	  }
      /* find the robots that want to change to the foraging task */
      for(std::pair<const std::string, SPiPuck>& c_pair : m_mapRobots) {
         SPiPuck& sPiPuck = c_pair.second;
         //~ const std::string& strId = c_pair.first;
         /* only consider robots currently performing the construction task */
         if(sPiPuck.Entity != nullptr) {
            /* to keep things simple: a non-empty set_task buffer indicates
               that the controller wants to swap to the foraging task */
            const std::string& strSetTaskBuffer =
               sPiPuck.Entity->GetDebugEntity().GetBuffer("set_task");
				std::stringstream cTask(strSetTaskBuffer);
				cTask >> strTask;
            if(strTask=="foraging") {
               RemoveEntity(*sPiPuck.Entity);
               sPiPuck.Entity = nullptr;
               sPiPuck.StepsUntilReturnToConstructionTask =
                  GetSimulator().GetRNG()->Poisson(m_fForagingDurationMean);
               // START HACK
               GetSimulator().GetMedium<CRadioMedium>("wifi").Update();
               // END HACK
            }
         }
      }
      /* find the robots that want to change to the construction task */
      for(std::pair<const std::string, SPiPuck>& c_pair : m_mapRobots) {
         SPiPuck& sPiPuck = c_pair.second;
         const std::string& strId = c_pair.first;
         /* only consider robots currently performing the construction task */
         if(sPiPuck.Entity == nullptr) {
            if(sPiPuck.StepsUntilReturnToConstructionTask == 0) {
               /* respawn robot with random position and orientation */
               //~ std::cerr << strId << ": foraging -> construction" << std::endl;
               CRange<Real> cXRange(0.05, cArenaSize.GetX() - 0.05);
               CRange<Real> cYRange(0.05, cArenaSize.GetY() - 0.05);
               for(;;) {
                  Real fX = GetSimulator().GetRNG()->Uniform(cXRange);
                  Real fY = GetSimulator().GetRNG()->Uniform(cYRange);
                  CRadians cZ = GetSimulator().GetRNG()->Uniform(CRadians::SIGNED_RANGE);
                  CVector3 cPosition(fX, fY, 0);
                  CQuaternion cOrientation(cZ, CVector3::Z);
                  sPiPuck.Entity = 
                     new CPiPuckEntity(strId, sPiPuck.Controller, cPosition, cOrientation);
                  AddEntity(*sPiPuck.Entity);
                  if(sPiPuck.Entity->GetEmbodiedEntity().IsCollidingWithSomething()) {
                     RemoveEntity(*sPiPuck.Entity);
                     sPiPuck.Entity = nullptr;
                  }
                  else {
                     /* initialize the previous cell coordinates of the new robot */
                     sPiPuck.PreviousX = static_cast<UInt32>(fX * m_arrGridLayout.at(0) / cArenaSize.GetX());
                     sPiPuck.PreviousY = static_cast<UInt32>(fY * m_arrGridLayout.at(1) / cArenaSize.GetY());
                     CCI_Controller& cController =
                        sPiPuck.Entity->GetControllableEntity().GetController();
                     CDTAAbstractWifiActuator* pcWifiActuator =
                        cController.GetActuator<CDTAAbstractWifiActuator>("wifi");
                     pcWifiActuator->SetCanSendTo(sPiPuck.CanSendTo);
                     // START HACK
                     GetSimulator().GetMedium<CRadioMedium>("wifi").Update();
                     // END HACK
                     break;
                  }
               }
               if(time > 5 && true){
               // INSTANTLY SHADE A RANDOM CELL 
				 CRange<UInt32> cXRangeInt(0, std::round(m_arrGridLayout[0]/4.0));
				 CRange<UInt32> cYRangeInt(0, std::round(m_arrGridLayout[1]/4.0));
				 int count_tries = 0;
				 if(unShadedCells < m_arrGridLayout[0] * m_arrGridLayout[1] && true) {
					for(;;) {
					   UInt32 unX = GetSimulator().GetRNG()->Uniform(cXRangeInt);
					   UInt32 unY = GetSimulator().GetRNG()->Uniform(cYRangeInt);
					   count_tries++;
					   if(!m_vecCells.at(unX + m_arrGridLayout[0] * unY)) {
						  m_vecCells[unX + m_arrGridLayout[0] * unY] = true;
						  break;
					   }
					   else if(count_tries > 250){
						   break;
					   }
					}
				 }
				 if(unShadedCells < m_arrGridLayout[0] * m_arrGridLayout[1] && count_tries > 250) {
					CRange<UInt32> cXRangeFullInt(0, std::round(m_arrGridLayout[0]));
					CRange<UInt32> cYRangeFullInt(0, std::round(m_arrGridLayout[1]));
					for(;;) {
					   UInt32 unX = GetSimulator().GetRNG()->Uniform(cXRangeFullInt);
					   UInt32 unY = GetSimulator().GetRNG()->Uniform(cYRangeFullInt);
					   if(!m_vecCells.at(unX + m_arrGridLayout[0] * unY)) {
						  m_vecCells[unX + m_arrGridLayout[0] * unY] = true;
						  break;
					   }
					}
				 }
			   }
              GetSpace().GetFloorEntity().SetChanged();
            }
            else {
               /* decrement counter */
               sPiPuck.StepsUntilReturnToConstructionTask -= 1;
            }
         }
      }
   }

   /****************************************/
   /****************************************/

   REGISTER_LOOP_FUNCTIONS(CDTAAbstractLoopFunctions, "dta_abstract_loop_functions");

   /****************************************/
   /****************************************/

}
