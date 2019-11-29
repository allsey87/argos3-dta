#include "dta_abstract_loop_functions.h"

#include <argos3/core/simulator/entity/floor_entity.h>
#include <argos3/plugins/simulator/entities/debug_entity.h>
#include <argos3/plugins/robots/pi-puck/simulator/pipuck_entity.h>

namespace argos {

   /****************************************/
   /****************************************/

   CDTAAbstractLoopFunctions::CDTAAbstractLoopFunctions() :
      m_cSpace(CSimulator::GetInstance().GetSpace()),
      m_pcRNG(CRandom::CreateRNG("argos")),
      m_unStepsUntilNextCellOccupied(0) {}

   /****************************************/
   /****************************************/

   void CDTAAbstractLoopFunctions::Init(TConfigurationNode& t_tree) {
      /* parse the layout */
      std::string strLayout;
      GetNodeAttribute(t_tree, "layout", strLayout);
      ParseValues<UInt32>(strLayout, 2, m_arrLayout.data(), ',');
      /* initialize vector of cells */
      m_vecOccupiedCells.assign(m_arrLayout[0] * m_arrLayout[1], false);
      m_cSpace.GetFloorEntity().SetChanged();
      /* create a map of the pi-puck robots */
      using TValueType = std::pair<const std::string, CAny>;
      for(const TValueType& t_robot : m_cSpace.GetEntitiesByType("pipuck")) {
         m_vecRobots.push_back(any_cast<CPiPuckEntity*>(t_robot.second));
      }
   }

   /****************************************/
   /****************************************/

   void CDTAAbstractLoopFunctions::Reset() {
      m_vecOccupiedCells.clear();
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
      return m_vecOccupiedCells.at(unX + m_arrLayout[0] * unY) ? CColor::GRAY50 : CColor::GRAY80;
   }

   /****************************************/
   /****************************************/

   void CDTAAbstractLoopFunctions::PostStep() {
      /* handle cell shading */
      if(m_unStepsUntilNextCellOccupied == 0) {
         CRange<UInt32> cXRange(0, m_arrLayout[0]);
         CRange<UInt32> cYRange(0, m_arrLayout[1]);
         UInt32 unOccupiedCells =
            std::count(std::begin(m_vecOccupiedCells), std::end(m_vecOccupiedCells), true);
         if(unOccupiedCells < m_arrLayout[0] * m_arrLayout[1]) {
            for(;;) {
               UInt32 unX = m_pcRNG->Uniform(cXRange);
               UInt32 unY = m_pcRNG->Uniform(cYRange);
               if(!m_vecOccupiedCells.at(unX + m_arrLayout[0] * unY)) {
                  m_vecOccupiedCells[unX + m_arrLayout[0] * unY] = true;
                  m_cSpace.GetFloorEntity().SetChanged();
                  /* TODO set time delay for next cell to be shaded */
                  m_unStepsUntilNextCellOccupied = 0;
                  break;
               }
            }
         }
      }
      else {
         m_unStepsUntilNextCellOccupied -= 1;
      }
      /* handle cell unshading */
      const CVector3& cArenaSize = m_cSpace.GetArenaSize();
      for(CPiPuckEntity* pc_robot : m_vecRobots) {
         const CVector3& cPosition =
            pc_robot->GetEmbodiedEntity().GetOriginAnchor().Position;
         UInt32 unX = static_cast<UInt32>(cPosition.GetX() * m_arrLayout.at(0) / cArenaSize.GetX());
         UInt32 unY = static_cast<UInt32>(cPosition.GetY() * m_arrLayout.at(1) / cArenaSize.GetY());
         if(m_vecOccupiedCells.at(unX + m_arrLayout[0] * unY)) {
            m_vecOccupiedCells[unX + m_arrLayout[0] * unY] = false;
            m_cSpace.GetFloorEntity().SetChanged();
         }
      }
      /* find the robots that want to switch to the foraging task */
      std::vector<CPiPuckEntity*>::iterator itEraseStartRange =
         std::remove_if(std::begin(m_vecRobots),
                        std::end(m_vecRobots),
                        [this] (CPiPuckEntity* pc_robot) {
            const std::string& strLoopFunctionsBuffer =
               pc_robot->GetDebugEntity().GetBuffer("loop_functions");
            if(!strLoopFunctionsBuffer.empty()) {
               /* remove robot from the space */
               CallEntityOperation<CSpaceOperationRemoveEntity, CSpace, void>(m_cSpace, *pc_robot);
               return true;
            }
            return false;
         });
      /* erase robots from the collection */
      m_vecRobots.erase(itEraseStartRange, std::end(m_vecRobots));
   }

   /****************************************/
   /****************************************/

   REGISTER_LOOP_FUNCTIONS(CDTAAbstractLoopFunctions, "dta_abstract_loop_functions");

}
