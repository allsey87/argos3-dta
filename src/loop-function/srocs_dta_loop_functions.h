#ifndef SROCS_DTA_LOOP_FUNCTIONS_H
#define SROCS_DTA_LOOP_FUNCTIONS_H

#include <argos3/core/simulator/space/space.h>
#include <argos3/core/simulator/loop_functions.h>

#include <argos3/core/utility/math/vector3.h>
#include <argos3/core/utility/math/range.h>

namespace argos {

   class CSRoCSDTALoopFunctions : public CLoopFunctions {

   public:

      CSRoCSDTALoopFunctions();

      virtual ~CSRoCSDTALoopFunctions() {}

      virtual void Init(TConfigurationNode& t_tree);

      //virtual void Reset();

      //virtual void Destroy();

      virtual void PreStep();

      //virtual void PostStep();

   private:

      CSpace& m_cSpace;
      CVector2 m_cSiteMinCorner;
      CVector2 m_cSiteMaxCorner;

   };


}

#endif

