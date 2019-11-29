#ifndef DTA_ABSTRACT_LOOP_FUNCTIONS_H
#define DTA_ABSTRACT_LOOP_FUNCTIONS_H

namespace argos {
   class CPiPuckEntity;
}

#include <argos3/core/simulator/space/space.h>
#include <argos3/core/simulator/loop_functions.h>
#include <argos3/core/utility/math/rng.h>

//#include <argos3/core/utility/math/vector3.h>
//#include <argos3/core/utility/math/range.h>

namespace argos {

   class CDTAAbstractLoopFunctions : public CLoopFunctions {

   public:

      CDTAAbstractLoopFunctions();

      virtual ~CDTAAbstractLoopFunctions() {}

      virtual void Init(TConfigurationNode& t_tree);

      virtual CColor GetFloorColor(const CVector2& c_position);

      virtual void Reset();

      //virtual void Destroy();

      //virtual void PreStep();

      virtual void PostStep();

   private:
      CSpace& m_cSpace;

      CRandom::CRNG* m_pcRNG;

      UInt32 m_unStepsUntilNextCellOccupied;

      std::vector<bool> m_vecOccupiedCells;

      std::array<UInt32, 2> m_arrLayout;

      std::vector<CPiPuckEntity*> m_vecRobots;

   };


}

#endif

