#include "srocs_dta_loop_functions.h"

#include <argos3/plugins/simulator/entities/block_entity.h>

namespace argos {

   /****************************************/
   /****************************************/

   CSRoCSDTALoopFunctions::CSRoCSDTALoopFunctions() :
      m_cSpace(CSimulator::GetInstance().GetSpace()) {}

   /****************************************/
   /****************************************/

   // TODO get number of blocks to keep in foraging area
   void CSRoCSDTALoopFunctions::Init(TConfigurationNode& t_tree) {
      GetNodeAttribute(t_tree, "site_min_corner", m_cSiteMinCorner);
      GetNodeAttribute(t_tree, "site_max_corner", m_cSiteMaxCorner);
   }

   /****************************************/
   /****************************************/

   void CSRoCSDTALoopFunctions::PreStep() {
      CSpace::TMapPerType& mapBlocks = m_cSpace.GetEntitiesByType("block");

      UInt32 unBlocksInConstructionArea = 0;
      UInt32 unBlocksInForagingArea = 0;
      /* loop over all the blocks */
      using TValueType = std::pair<const std::string, CAny>;
      for(const TValueType& t_block : mapBlocks) {
         CBlockEntity* pcBlock = any_cast<CBlockEntity*>(t_block.second);
         SAnchor& sAnchor = pcBlock->GetEmbodiedEntity().GetOriginAnchor();
         std::cerr << sAnchor.Position << std::endl;
         if(sAnchor.Position.GetX() < m_cSiteMaxCorner.GetX() &&
            sAnchor.Position.GetX() > m_cSiteMinCorner.GetX() &&
            sAnchor.Position.GetY() < m_cSiteMaxCorner.GetY() &&
            sAnchor.Position.GetY() > m_cSiteMinCorner.GetY()) {
            unBlocksInConstructionArea++;
         }
         else {
            unBlocksInForagingArea++;
         }
      }
      std::cerr << "unBlocksInConstructionArea = " << static_cast<int>(unBlocksInConstructionArea) << std::endl;
      std::cerr << "unBlocksInForagingArea = " << static_cast<int>(unBlocksInForagingArea) << std::endl;

   }

   /****************************************/
   /****************************************/

   REGISTER_LOOP_FUNCTIONS(CSRoCSDTALoopFunctions, "srocs_dta_loop_functions");

}
