#include "dta_abstract_loop_functions.h"

#include <argos3/core/simulator/entity/floor_entity.h>
#include <argos3/plugins/robots/pi-puck/simulator/pipuck_entity.h>

namespace argos {

   /****************************************/
   /****************************************/

   CDTAAbstractLoopFunctions::CDTAAbstractLoopFunctions() :
      m_cSpace(CSimulator::GetInstance().GetSpace()) {}

   /****************************************/
   /****************************************/

   void CDTAAbstractLoopFunctions::Init(TConfigurationNode& t_tree) {
      std::string strLayout;
      GetNodeAttribute(t_tree, "layout", strLayout);
      ParseValues<SInt32>(strLayout, 2, m_arrLayout.data(), ',');
      


      /* parse loop function configuration */
      //GetNodeAttribute(t_tree, "blocks_in_foraging_area", m_unTargetBlocksInForagingArea);
      /* calculate the corners of the arena */

      /*
      const CVector3& cArenaSize();
      const CVector3& cArenaCenter(m_cSpace.GetArenaCenter());    
      m_cArenaRangeX.Set(cArenaCenter.GetX() - (cArenaSize.GetX() * 0.5) + 0.1,
                         cArenaCenter.GetX() + (cArenaSize.GetX() * 0.5) - 0.1);
      m_cArenaRangeY.Set(cArenaCenter.GetY() - (cArenaSize.GetY() * 0.5) + 0.1,
                         cArenaCenter.GetY() + (cArenaSize.GetY() * 0.5) - 0.1);
      */

      /* create a map of the pi-puck robots */
      using TValueType = std::pair<const std::string, CAny>;
      for(const TValueType& t_robot : m_cSpace.GetEntitiesByType("pipuck")) {
         m_vecRobots.push_back(any_cast<CPiPuckEntity*>(t_robot.second));
      }
   }

   /****************************************/
   /****************************************/

   CColor CDTAAbstractLoopFunctions::GetFloorColor(const CVector2& c_position) {
      const CVector3& cArenaSize = m_cSpace.GetArenaSize();
      /* convert c_position into cell coordinates */
      SInt32 nX = 
         static_cast<SInt32>(c_position.GetX() * m_arrLayout.at(0) / cArenaSize.GetX());
      SInt32 nY =
         static_cast<SInt32>(c_position.GetY() * m_arrLayout.at(1) / cArenaSize.GetY());
      /* check if the cell is occupied */
      if(std::find_if(std::begin(m_vecOccupiedCells),
                      std::end(m_vecOccupiedCells),
                      [nX, nY] (const std::pair<SInt32, SInt32>& c_coords) {
                         return ((nX == c_coords.first) && (nY == c_coords.second));
                      }) != std::end(m_vecOccupiedCells)) {
         /* cell is occupied */
         return CColor::GRAY50;
      }
      else {
         /* cell is unoccupied */
         return CColor::GRAY80;
      }
   }

   /****************************************/
   /****************************************/

   void CDTAAbstractLoopFunctions::PostStep() {
      const CVector3& cArenaSize = m_cSpace.GetArenaSize();
      std::vector<std::pair<SInt32, SInt32> > m_vecPrevOccupiedCells;
      m_vecPrevOccupiedCells.swap(m_vecOccupiedCells);
      /* loop over all the robots */
      for(CPiPuckEntity* pc_robot : m_vecRobots) {
         SAnchor& sAnchor = pc_robot->GetEmbodiedEntity().GetOriginAnchor();
         /* calculate cell positions for the robots */
         SInt32 nX = 
            static_cast<SInt32>(std::floor(sAnchor.Position.GetX() * m_arrLayout.at(0) / cArenaSize.GetX()));
         SInt32 nY =
            static_cast<SInt32>(std::floor(sAnchor.Position.GetY() * m_arrLayout.at(1) / cArenaSize.GetY()));
         m_vecOccupiedCells.emplace_back(nX, nY);
         //LOG << pc_robot->GetId() << ": " << static_cast<int>(nX) << ", " << static_cast<int>(nY) << std::endl;
      }
      std::sort(std::begin(m_vecOccupiedCells),
                std::end(m_vecOccupiedCells),
                [] (const std::pair<SInt32, SInt32> c_lhs,
                    const std::pair<SInt32, SInt32> c_rhs) {
         return (c_lhs.first == c_rhs.first) ? 
                (c_lhs.second < c_rhs.second) :
                (c_lhs.first < c_rhs.first);
      });

      if(!std::equal(std::begin(m_vecOccupiedCells),
                     std::end(m_vecOccupiedCells),
                     std::begin(m_vecPrevOccupiedCells),
                     std::end(m_vecPrevOccupiedCells))) {
         /* the list of cells occupied by the robots has changed, update the floor */
         std::cerr << "!" << std::endl;
         m_cSpace.GetFloorEntity().SetChanged();
      }
   }

   /****************************************/
   /****************************************/

   REGISTER_LOOP_FUNCTIONS(CDTAAbstractLoopFunctions, "dta_abstract_loop_functions");

}
