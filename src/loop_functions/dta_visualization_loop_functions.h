#ifndef DTA_VISUALIZATION_LOOP_FUNCTIONS_H
#define DTA_VISUALIZATION_LOOP_FUNCTIONS_H

namespace argos {
   class CBlockEntity;
   class CBuilderBotEntity;
}

#include <argos3/core/simulator/space/space.h>
#include <argos3/core/simulator/loop_functions.h>

#include <argos3/core/utility/math/vector3.h>
#include <argos3/core/utility/math/range.h>

namespace argos {

   class CDTAVisualizationLoopFunctions : public CLoopFunctions {

   public:

      CDTAVisualizationLoopFunctions();

      virtual ~CDTAVisualizationLoopFunctions() {}

      virtual void Init(TConfigurationNode& t_tree);

      //virtual void Reset();

      //virtual void Destroy();

      virtual void PreStep();

      //virtual void PostStep();

      virtual CColor GetFloorColor(const CVector2& c_position);

   private:
      CSpace& m_cSpace;

      TConfigurationNode m_tBlockTemplateNode;

      CRange<Real> m_cArenaRangeX;
      CRange<Real> m_cArenaRangeY;
      CRange<Real> m_cRotationRange;

      CVector2 m_cConstructionSiteMinCorner;
      CVector2 m_cConstructionSiteMaxCorner;

      UInt32 m_unTargetBlocksInForagingArea;

      std::vector<CBuilderBotEntity*> m_vecRobots;
      std::vector<CBlockEntity*> m_vecBlocks;





   };


}

#endif

