#include "dta_abstract_loop_functions.h"

#include <argos3/core/simulator/entity/floor_entity.h>
#include <argos3/plugins/simulator/entities/debug_entity.h>
#include <argos3/plugins/simulator/media/radio_medium.h>
#include <argos3/plugins/robots/generic/simulator/wifi_default_actuator.h>
#include <argos3/plugins/robots/pi-puck/simulator/pipuck_entity.h>

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
      m_cSpace(CSimulator::GetInstance().GetSpace()),
      m_pcRNG(CRandom::CreateRNG("argos")),
      m_unStepsUntilShadeCell(0) {}

   /****************************************/
   /****************************************/

   void CDTAAbstractLoopFunctions::Init(TConfigurationNode& t_tree) {
      /* parse the layout */
      std::string strLayout;
      GetNodeAttribute(GetNode(t_tree, "grid"), "layout", strLayout);
      ParseValues<UInt32>(strLayout, 2, m_arrLayout.data(), ',');
      /* initialize vector of cells */
      m_vecCells.assign(m_arrLayout[0] * m_arrLayout[1], false);
      m_cSpace.GetFloorEntity().SetChanged();
      /* create a map of the pi-puck robots */
      TConfigurationNodeIterator itPiPuck("pipuck");
      std::string strId;
      std::string strController;
      std::string strCanSendTo;
      std::vector<std::string> vecCanSendTo;
      std::set<std::string> setCanSendTo;
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
   }

   /****************************************/
   /****************************************/

   void CDTAAbstractLoopFunctions::Reset() {
      /* remove all robots */
      for(std::pair<const std::string, SPiPuck> c_pair : m_mapRobots) {
         SPiPuck& s_pipuck = c_pair.second;
         RemoveEntity(*s_pipuck.Entity);
         s_pipuck.Entity = nullptr;
         s_pipuck.StepsUntilReturnToConstructionTask = 0;
      }
      /* clear all cells and mark the floor as changed */
      m_vecCells.assign(m_vecCells.size(), false);
      m_cSpace.GetFloorEntity().SetChanged();
   }

   /****************************************/
   /****************************************/

   CColor CDTAAbstractLoopFunctions::GetFloorColor(const CVector2& c_position) {
      const CVector3& cArenaSize = m_cSpace.GetArenaSize();
      /* convert c_position into cell coordinates */
      UInt32 unX = static_cast<UInt32>(c_position.GetX() * m_arrLayout.at(0) / cArenaSize.GetX());
      UInt32 unY = static_cast<UInt32>(c_position.GetY() * m_arrLayout.at(1) / cArenaSize.GetY());
      /* shade cell if occupied */
      return m_vecCells.at(unX + m_arrLayout[0] * unY) ? CColor::GRAY50 : CColor::GRAY80;
   }

   /****************************************/
   /****************************************/

   void CDTAAbstractLoopFunctions::PostStep() {
      /* handle cell shading */
      if(m_unStepsUntilShadeCell == 0) {
         CRange<UInt32> cXRange(0, m_arrLayout[0]);
         CRange<UInt32> cYRange(0, m_arrLayout[1]);
         UInt32 unShadedCells =
            std::count(std::begin(m_vecCells), std::end(m_vecCells), true);
         if(unShadedCells < m_arrLayout[0] * m_arrLayout[1]) {
            for(;;) {
               UInt32 unX = m_pcRNG->Uniform(cXRange);
               UInt32 unY = m_pcRNG->Uniform(cYRange);
               if(!m_vecCells.at(unX + m_arrLayout[0] * unY)) {
                  m_vecCells[unX + m_arrLayout[0] * unY] = true;
                  m_cSpace.GetFloorEntity().SetChanged();
                  // TODO the following delay is for the next cell to be shaded
                  m_unStepsUntilShadeCell = 0;
                  break;
               }
            }
         }
      }
      else {
         m_unStepsUntilShadeCell -= 1;
      }
      /* handle cell unshading */
      const CVector3& cArenaSize = m_cSpace.GetArenaSize();
      for(CPiPuckEntity* pc_robot : m_vecRobots) {
         const CVector3& cPosition =
            pc_robot->GetEmbodiedEntity().GetOriginAnchor().Position;
         UInt32 unX = static_cast<UInt32>(cPosition.GetX() * m_arrLayout.at(0) / cArenaSize.GetX());
         UInt32 unY = static_cast<UInt32>(cPosition.GetY() * m_arrLayout.at(1) / cArenaSize.GetY());
         if(m_vecCells.at(unX + m_arrLayout[0] * unY)) {
            m_vecCells[unX + m_arrLayout[0] * unY] = false;
            m_cSpace.GetFloorEntity().SetChanged();
         }
      }
      /* find the robots that want to change to the foraging task */
      for(std::pair<const std::string, SPiPuck>& c_pair : m_mapRobots) {
         SPiPuck& sPiPuck = c_pair.second;
         const std::string& strId = c_pair.first;
         /* only consider robots currently performing the construction task */
         if(sPiPuck.Entity != nullptr) {
            /* to keep things simple: a non-empty loop_functions buffer indicates
               that the controller wants to swap to the foraging task */
            const std::string& strLoopFunctionsBuffer =
               sPiPuck.Entity->GetDebugEntity().GetBuffer("loop_functions");
            if(!strLoopFunctionsBuffer.empty()) {
               std::cerr << strId << ": construction -> foraging" << std::endl;
               RemoveEntity(*sPiPuck.Entity);
               // START HACK
               CSimulator::GetInstance().GetMedium<CRadioMedium>("wifi").Update();
               // END HACK
               sPiPuck.Entity = nullptr;
               // TODO the follow value control when the robots will be respawned
               sPiPuck.StepsUntilReturnToConstructionTask = 5;
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
               std::cerr << strId << ": foraging -> construction" << std::endl;
               CRange<Real> cXRange(0.05, cArenaSize.GetX() - 0.05);
               CRange<Real> cYRange(0.05, cArenaSize.GetY() - 0.05);
               for(;;) {
                  Real fX = CSimulator::GetInstance().GetRNG()->Uniform(cXRange);
                  Real fY = CSimulator::GetInstance().GetRNG()->Uniform(cYRange);
                  CRadians cZ = CSimulator::GetInstance().GetRNG()->Uniform(CRadians::SIGNED_RANGE);
                  CVector3 cPosition(fX, fY, 0);
                  CQuaternion cOrientation(cZ, CVector3::Z);
                  sPiPuck.Entity = 
                     new CPiPuckEntity(strId, sPiPuck.Controller, cPosition, cOrientation);
                  AddEntity(*sPiPuck.Entity);
                  if(sPiPuck.Entity->GetEmbodiedEntity().IsCollidingWithSomething()) {
                     RemoveEntity(*sPiPuck.Entity);
                  }
                  else {
                     CCI_Controller& cController =
                        sPiPuck.Entity->GetControllableEntity().GetController();
                     CDTAAbstractWifiActuator* pcWifiActuator =
                        cController.GetActuator<CDTAAbstractWifiActuator>("wifi");
                     pcWifiActuator->SetCanSendTo(sPiPuck.CanSendTo);
                     // START HACK
                     CSimulator::GetInstance().GetMedium<CRadioMedium>("wifi").Update();
                     // END HACK
                     break;
                  }
               }
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