#ifndef DTA_ABSTRACT_LOOP_FUNCTIONS_H
#define DTA_ABSTRACT_LOOP_FUNCTIONS_H

namespace argos {
   class CPiPuckEntity;
}

#include <argos3/core/simulator/space/space.h>
#include <argos3/core/simulator/loop_functions.h>

//#include <argos3/core/utility/math/vector3.h>
//#include <argos3/core/utility/math/range.h>

namespace argos {

   class CDTAAbstractLoopFunctions : public CLoopFunctions {

   public:

      CDTAAbstractLoopFunctions();

      virtual ~CDTAAbstractLoopFunctions() {}

      virtual void Init(TConfigurationNode& t_tree);

      virtual CColor GetFloorColor(const CVector2& c_position);

      //virtual void Reset();

      //virtual void Destroy();

      //virtual void PreStep();

      virtual void PostStep();

   private:
      CSpace& m_cSpace;

      std::vector<CPiPuckEntity*> m_vecRobots;

      std::vector<std::pair<SInt32, SInt32> > m_vecOccupiedCells;

      std::array<SInt32, 2> m_arrLayout;


   };


}

#endif

