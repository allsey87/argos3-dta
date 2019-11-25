#ifndef DTA_SROCS_LOOP_FUNCTIONS_H
#define DTA_SROCS_LOOP_FUNCTIONS_H

namespace argos {
   class CBlockEntity;
   class CBuilderBotEntity;
}

#include <argos3/core/simulator/space/space.h>
#include <argos3/core/simulator/loop_functions.h>

#include <argos3/core/utility/math/vector3.h>
#include <argos3/core/utility/math/range.h>

namespace argos {

   class CDTASRoCSLoopFunctions : public CLoopFunctions {

   public:

      CDTASRoCSLoopFunctions();

      virtual ~CDTASRoCSLoopFunctions() {}

      virtual void Init(TConfigurationNode& t_tree);

      //virtual void Reset();

      //virtual void Destroy();

      virtual void PreStep();

      //virtual void PostStep();

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

