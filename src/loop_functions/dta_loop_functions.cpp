#include "dta_loop_functions.h"

#include <argos3/core/simulator/entity/floor_entity.h>
#include <argos3/core/utility/math/rng.h>
#include <argos3/plugins/simulator/entities/debug_entity.h>
#include <argos3/plugins/simulator/media/radio_medium.h>
#include <argos3/plugins/robots/generic/simulator/wifi_default_actuator.h>
#include <argos3/plugins/robots/pi-puck/simulator/pipuck_entity.h>

#define CSV_HEADER "\"foraging robots\"\t\"building robots\"\t\"average estimate\"\t\"average deviation\"\t\"construction events\"\t\"blocks in cache\"\t\"average degree\""

#define WALL_THICKNESS 0.025
#define MAX_ATTEMPTS 1000

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

      /****************************************/
      /****************************************/

   private:
      std::set<std::string> m_setCanSendTo;
   };

   /****************************************/
   /****************************************/
   
   class CDTAProximityWifiActuator : public CWifiDefaultActuator {
   public:
      class CTxOperation : public CPositionalIndex<CRadioEntity>::COperation {
      public:
         CTxOperation(const CRadioEntity& c_tx_radio,
                      const std::list<CByteArray>& lst_messages,
                      Real f_range) :
            m_cTxRadio(c_tx_radio),
            m_lstMessages(lst_messages),
            m_fRange(f_range) {}

         virtual bool operator()(CRadioEntity& c_rx_radio) {
            if(&c_rx_radio != &m_cTxRadio) {
               const CVector3& cRxRadioPosition = c_rx_radio.GetPosition();
               const CVector3& cTxRadioPosition = m_cTxRadio.GetPosition();
               Real fDistance = (cRxRadioPosition - cTxRadioPosition).Length();
               if(fDistance < m_fRange) {
                  for(const CByteArray& c_message : m_lstMessages) {
                     c_rx_radio.ReceiveData(cTxRadioPosition, c_message);
                  }
               }
            }
            return true;
         }

      private:
         const CRadioEntity& m_cTxRadio;
         const std::list<CByteArray>& m_lstMessages;
         Real m_fRange = 0.0;
      };

      /****************************************/
      /****************************************/

      virtual void Init(TConfigurationNode& t_tree) override {
         std::string strRange;
         GetNodeAttribute(t_tree, "range", strRange);
         if(strRange == "inf") {
            m_bLimitRange = false;
            m_fRange = std::numeric_limits<Real>::infinity();
         }
         else {
            m_bLimitRange = true;
            GetNodeAttribute(t_tree, "range", m_fRange);
         }
         /* initialize base class */
         CWifiDefaultActuator::Init(t_tree);
      }

      /****************************************/
      /****************************************/

      virtual void Update() override {
         if(!m_lstMessages.empty()) {
            /* Create operation instance */
            CTxOperation cTxOperation(*m_pcRadioEntity, m_lstMessages, m_fRange);
            /* Get positional index */
            CPositionalIndex<CRadioEntity>* pcRadioIndex =
               &(m_pcRadioEntity->GetMedium().GetIndex());
            if(m_bLimitRange) {
               /* Calculate the range of the transmitting radio */
               CVector3 cTxRange(m_fRange, m_fRange, m_fRange);
               /* Transmit the data to receiving radios in the space */
               pcRadioIndex->ForEntitiesInBoxRange(m_pcRadioEntity->GetPosition(), cTxRange, cTxOperation);
            }
            else {
               pcRadioIndex->ForAllEntities(cTxOperation);
            }
            /* Flush data from the control interface */
            m_lstMessages.clear();
         }
      }

      /****************************************/
      /****************************************/

   private:
      bool m_bLimitRange = false;
      Real m_fRange = 0.0;

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
   
   REGISTER_ACTUATOR(CDTAProximityWifiActuator,
                     "wifi", "dta_proximity",
                     "Michael Allwright [allsey87@gmail.com]",
                     "1.0",
                     "A proximity-based wifi actuator for the DTA experiments.",
                     "This actuator sends messages over wifi.",
                     "Usable"
   );

   /****************************************/
   /****************************************/

   CDTALoopFunctions::CDTALoopFunctions() :
      m_arrGridLayout{0.0, 0.0},
      m_fMeanForagingDurationInitial(0.0),
      m_fMeanForagingDurationGradient(0.0),
      m_unConstructionLimit(0),
      m_eShadingDistribution(EShadingDistribution::UNIFORM),
      m_pcOutput(nullptr) {
      /* calculate the ticks per second */
      UInt32 unTicksPerSecond =
         static_cast<UInt32>(std::round(CPhysicsEngine::GetInverseSimulationClockTick()));
      m_vecConstructionEvents.assign(unTicksPerSecond, 0);
   }

   /****************************************/
   /****************************************/

   void CDTALoopFunctions::Init(TConfigurationNode& t_tree) {
      GetNodeAttributeOrDefault(t_tree, "output", m_strOutputFilename, m_strOutputFilename);
      if(m_strOutputFilename.empty()) {
         m_pcOutput = &std::cout;
      }
      else {
         std::ofstream* pcOutputFile = 
            new std::ofstream(m_strOutputFilename, std::ios_base::out | std::ios_base::trunc);
         if(!pcOutputFile->is_open()) {
            THROW_ARGOSEXCEPTION("Can not create output file \"" << m_strOutputFilename << "\"");
         }
         m_pcOutput = pcOutputFile;
      }
      if(!m_pcOutput->good()) {
         THROW_ARGOSEXCEPTION("Output stream is not ready");
      }
      *m_pcOutput << CSV_HEADER << std::endl;
      /* parse the parameters */
      TConfigurationNode& tParameters = GetNode(t_tree, "parameters");
      /* parse the foraging delay coefficient and the construction limit */
      GetNodeAttribute(tParameters, "mean_foraging_duration_initial", m_fMeanForagingDurationInitial);
      GetNodeAttribute(tParameters, "mean_foraging_duration_gradient", m_fMeanForagingDurationGradient);
      GetNodeAttribute(tParameters, "construction_limit", m_unConstructionLimit);
      std::string strShadingDistribution;
      GetNodeAttribute(tParameters, "shading_distribution", strShadingDistribution);
      if(strShadingDistribution == "biased") {
         m_eShadingDistribution = EShadingDistribution::BIASED;
      }
      else if(strShadingDistribution == "uniform") {
         m_eShadingDistribution = EShadingDistribution::UNIFORM;
      }
      else {
         THROW_ARGOSEXCEPTION("Unsupported shading distribution");
      }
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
      /* parse the pipuck robots */
      for(itPiPuck = itPiPuck.begin(&GetNode(t_tree,"robots"));
         itPiPuck != itPiPuck.end();
         ++itPiPuck) {
         strId.clear();
         strController.clear();
         strCanSendTo.clear();
         vecCanSendTo.clear();
         setCanSendTo.clear();
         GetNodeAttribute(*itPiPuck, "id", strId);
         GetNodeAttribute(*itPiPuck, "controller", strController);
         GetNodeAttributeOrDefault(*itPiPuck, "can_send_to", strCanSendTo, strCanSendTo);
         Tokenize(strCanSendTo, vecCanSendTo, ",");
         setCanSendTo.insert(std::begin(vecCanSendTo),
                             std::end(vecCanSendTo));
         m_mapRobots.emplace(std::piecewise_construct,
                             std::forward_as_tuple(strId),
                             std::forward_as_tuple(strController, setCanSendTo));
      }
   }

   /****************************************/
   /****************************************/

   void CDTALoopFunctions::Reset() {
      /* remove all robots */
      for(std::pair<const std::string, SPiPuck>& c_pair : m_mapRobots) {
         SPiPuck& s_pipuck = c_pair.second;
         RemoveEntity(*s_pipuck.Entity);
         s_pipuck.Entity = nullptr;
         s_pipuck.StepsUntilReturnToConstructionTask = 0;
      }
      /* clear all cells and mark the floor as changed */
      m_vecCells.assign(m_vecCells.size(), false);
      GetSpace().GetFloorEntity().SetChanged();
      /* zero out the construction events */
      m_vecConstructionEvents.assign(m_vecConstructionEvents.size(), 0);
      /* reconfigure the output file */
      if(m_pcOutput != &std::cout) {
         std::ofstream* pcOutputFile = static_cast<std::ofstream*>(m_pcOutput);
         if(pcOutputFile->is_open()) {
            pcOutputFile->close();
         }
         delete pcOutputFile;
         pcOutputFile = 
            new std::ofstream(m_strOutputFilename, std::ios_base::out | std::ios_base::trunc);
         if(!pcOutputFile->is_open()) {
            THROW_ARGOSEXCEPTION("Can not create output file \"" << m_strOutputFilename << "\"");
         }
         m_pcOutput = pcOutputFile;
      }
      m_pcOutput->clear();
      if(!m_pcOutput->good()) {
         THROW_ARGOSEXCEPTION("Output stream is not ready");
      }
      *m_pcOutput << CSV_HEADER << std::endl;
   }

   /****************************************/
   /****************************************/

   void CDTALoopFunctions::Destroy() {
      if(m_pcOutput != &std::cout) {
         delete m_pcOutput;
      }
   }

   /****************************************/
   /****************************************/

   CColor CDTALoopFunctions::GetFloorColor(const CVector2& c_position) {
      if(IsOnGrid(c_position)) {
         const std::pair<UInt32, UInt32>& cCoordinates = 
            GetGridCoordinatesFor(c_position);
         /* shade cell if occupied */
         return m_vecCells.at(cCoordinates.first + m_arrGridLayout[0] * cCoordinates.second) ?
            CColor::GRAY50 : CColor::GRAY80;
      }
      else {
         return CColor::WHITE;
      }
   }

   /****************************************/
   /****************************************/

   void CDTALoopFunctions::PostStep() {
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
      UInt32 unExploringRobotsCount = 0;
      /* calculate the number of shaded cells */
      UInt32 unShadedCells =
         std::count(std::begin(m_vecCells), std::end(m_vecCells), true);
      /* get the current estimate value from each building robot */
	   Real fEstimate = 0.0,
         avg_fEstimate = 0.0, fDeviation = 0.0, avg_fDeviation = 0.0, fDegree = 0.0, avg_fDegree = 0.0;
      if(unBuildingRobotsCount > 0){
		   for(std::pair<const std::string, SPiPuck>& c_pair : m_mapRobots) {
			   SPiPuck& sPiPuck = c_pair.second;
			   /* only consider robots currently performing the construction task */
			   if(sPiPuck.Entity != nullptr) {
               const std::string& strSetTaskBuffer =
                  sPiPuck.Entity->GetDebugEntity().GetBuffer("set_task");
               if(strSetTaskBuffer.find("exploring") != std::string::npos) {
                  /* read estimate from robot */
                  const std::string& strEstimateBuffer =
                     sPiPuck.Entity->GetDebugEntity().GetBuffer("set_estimate");
                  std::istringstream(strEstimateBuffer) >> fEstimate;
                  avg_fEstimate += fEstimate;
                  /* read deviation from robot */
                  const std::string& strDeviationBuffer =
                     sPiPuck.Entity->GetDebugEntity().GetBuffer("set_deviation");
                  std::istringstream(strDeviationBuffer) >> fDeviation;
                  avg_fDeviation += fDeviation;
                  /* read degree from robot */
                  const std::string& strDegreeBuffer =
                     sPiPuck.Entity->GetDebugEntity().GetBuffer("set_degree");
                  std::istringstream(strDegreeBuffer) >> fDegree;
                  avg_fDegree += fDegree;
                  /* increment the count of the exploring robots */
                  unExploringRobotsCount++;
				   }
			   }
		   }
	   }
	   UInt32 unTotalConstructionEvents = 0;
	   for(UInt32 un_count : m_vecConstructionEvents) {
		   unTotalConstructionEvents += un_count;
	   }
      /* print the results */
      if(unExploringRobotsCount != 0) {
		   avg_fEstimate = avg_fEstimate / static_cast<Real>(unExploringRobotsCount);
		   avg_fDeviation = avg_fDeviation / static_cast<Real>(unExploringRobotsCount);
		   avg_fDegree = avg_fDegree / static_cast<Real>(unExploringRobotsCount);
		   /* write output */
		   if(m_pcOutput->good()) {
			   *m_pcOutput << unForagingRobotsCount << '\t'
                        << unBuildingRobotsCount << '\t'
                        << avg_fEstimate << '\t'
                        << avg_fDeviation << '\t'
                        << unTotalConstructionEvents << '\t'
                        << unShadedCells / static_cast<Real>(m_arrGridLayout[0] * m_arrGridLayout[1]) << '\t'
                        << avg_fDegree 
                        << std::endl;
		   }
         else {
           THROW_ARGOSEXCEPTION("Output stream is not ready");
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
            const CVector3& cPosition = sPiPuck.Entity->GetEmbodiedEntity().GetOriginAnchor().Position;
            const std::pair<UInt32, UInt32>& cCoordinates =
               GetGridCoordinatesFor(CVector2(cPosition.GetX(), cPosition.GetY()));
            /* check if robot has moved to a different cell */
            if((cCoordinates.first != sPiPuck.PreviousX) || (cCoordinates.second != sPiPuck.PreviousY)) {
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
               sPiPuck.PreviousX = cCoordinates.first;
               sPiPuck.PreviousY = cCoordinates.second;
			   }
		   }
	   }
      /* find the robots that want to change to the foraging task */
      for(std::pair<const std::string, SPiPuck>& c_pair : m_mapRobots) {
         SPiPuck& sPiPuck = c_pair.second;
         /* only consider robots currently performing the construction task */
         if(sPiPuck.Entity != nullptr) {
            const std::string& strSetTaskBuffer =
               sPiPuck.Entity->GetDebugEntity().GetBuffer("set_task");
            if(strSetTaskBuffer.find("foraging") != std::string::npos) {
               RemoveEntity(*sPiPuck.Entity);
               sPiPuck.Entity = nullptr;
               Real fMeanForagingDuration = m_fMeanForagingDurationInitial +
                  m_fMeanForagingDurationGradient * GetSpace().GetSimulationClock();
               sPiPuck.StepsUntilReturnToConstructionTask =
                  GetSimulator().GetRNG()->Poisson(fMeanForagingDuration);
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
               /* respawn robot with random position and orientation (minus 5 cm from the boundary) */
               CRange<Real> cXRange(0.05, cArenaSize.GetX() - 0.05);
               CRange<Real> cYRange(0.05, cArenaSize.GetY() - 0.05);
               for(;;) {
                  Real fX = GetSimulator().GetRNG()->Uniform(cXRange);
                  Real fY = GetSimulator().GetRNG()->Uniform(cYRange);
                  CRadians cZ = GetSimulator().GetRNG()->Uniform(CRadians::SIGNED_RANGE);
                  CVector3 cPosition(fX, fY, 0);
                  CQuaternion cOrientation(cZ, CVector3::Z);
                  sPiPuck.Entity = 
                     new CPiPuckEntity(strId, sPiPuck.Controller, cPosition, cOrientation, false, "wifi");
                  AddEntity(*sPiPuck.Entity);
                  std::cerr << ".";
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
                     /* try to set the CanSendTo attribute of the CDTAAbstractWifiActuator. This will fail
                        silently if we are using a different actuator, e.g., CDTAProximityWifiActuator */
                     try {
                        CDTAAbstractWifiActuator* pcWifiActuator =
                           cController.GetActuator<CDTAAbstractWifiActuator>("wifi");
                        pcWifiActuator->SetCanSendTo(sPiPuck.CanSendTo);
                     }
                     catch(CARGoSException& ex) {}
                     // START HACK
                     GetSimulator().GetMedium<CRadioMedium>("wifi").Update();
                     // END HACK
                     break;
                  }
               }
               /* do not shade cells when the robots are added just after initialization */
               if(GetSpace().GetSimulationClock() > 5) {
                  /* only shade a cell if an unshaded cell exists */
                  if(unShadedCells < m_arrGridLayout[0] * m_arrGridLayout[1]) {
                     if(m_eShadingDistribution == EShadingDistribution::BIASED) {
                        Real fMeanX = WALL_THICKNESS + cArenaSize.GetX() / 4.0;
                        Real fMeanY = WALL_THICKNESS + cArenaSize.GetY() / 4.0;
                        for(;;) {
                           CVector2 cRandomPosition(GetSimulator().GetRNG()->Gaussian(fMeanX / 4.0, fMeanX),
                                                    GetSimulator().GetRNG()->Gaussian(fMeanY / 4.0, fMeanY));
                           if(IsOnGrid(cRandomPosition)) {
                              const std::pair<UInt32, UInt32>& cRandomCoordinates =
                                 GetGridCoordinatesFor(cRandomPosition);
                              if(!m_vecCells.at(cRandomCoordinates.first + m_arrGridLayout[0] * cRandomCoordinates.second)) {
                                 m_vecCells.at(cRandomCoordinates.first + m_arrGridLayout[0] * cRandomCoordinates.second) = true;
                                 break;
                              }
                           }
                        }
                     }
                     else if(m_eShadingDistribution == EShadingDistribution::UNIFORM) {
                        CRange<UInt32> cXRange(0, m_arrGridLayout[0]);
                        CRange<UInt32> cYRange(0, m_arrGridLayout[1]);
                        for(;;) {
                           UInt32 unX = GetSimulator().GetRNG()->Uniform(cXRange);
                           UInt32 unY = GetSimulator().GetRNG()->Uniform(cYRange);
                           if(m_vecCells.at(unX + m_arrGridLayout[0] * unY) == false) {
                              /* shade the unshaded tile */
                              m_vecCells[unX + m_arrGridLayout[0] * unY] = true;
                              break;
                           }
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

   bool CDTALoopFunctions::IsOnGrid(const CVector2& c_position) {
      const CVector3& cArenaSize = GetSpace().GetArenaSize();
      CRange<Real> cXRange(WALL_THICKNESS, cArenaSize.GetX() - WALL_THICKNESS);
      CRange<Real> cYRange(WALL_THICKNESS, cArenaSize.GetY() - WALL_THICKNESS);
      return (cXRange.WithinMinBoundIncludedMaxBoundIncluded(c_position.GetX()) &&
              cYRange.WithinMinBoundIncludedMaxBoundIncluded(c_position.GetY()));
   }

   /****************************************/
   /****************************************/

   std::pair<UInt32, UInt32> CDTALoopFunctions::GetGridCoordinatesFor(const CVector2& c_position) {
      const CVector3& cArenaSize = GetSpace().GetArenaSize();
      UInt32 unX = static_cast<UInt32>((c_position.GetX() - WALL_THICKNESS) * m_arrGridLayout.at(0) / (cArenaSize.GetX() - 2 * WALL_THICKNESS));
      UInt32 unY = static_cast<UInt32>((c_position.GetY() - WALL_THICKNESS) * m_arrGridLayout.at(1) / (cArenaSize.GetY() - 2 * WALL_THICKNESS));
      return std::pair<UInt32, UInt32>(unX, unY);
   }

   /****************************************/
   /****************************************/

   REGISTER_LOOP_FUNCTIONS(CDTALoopFunctions, "dta_loop_functions");

   /****************************************/
   /****************************************/

}
