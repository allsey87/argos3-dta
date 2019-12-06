#ifndef DTA_ABSTRACT_LOOP_FUNCTIONS_H
#define DTA_ABSTRACT_LOOP_FUNCTIONS_H

namespace argos {
   class CPiPuckEntity;
}

#include <argos3/core/simulator/space/space.h>
#include <argos3/core/simulator/loop_functions.h>
#include <argos3/core/utility/math/rng.h>

#include <set>

namespace argos {

   class CDTAAbstractLoopFunctions : public CLoopFunctions {

   public:

      struct SPiPuck {
         SPiPuck(const std::string& str_controller,
                 const std::set<std::string>& set_can_send_to) :
            Entity(nullptr),
            StepsUntilReturnToConstructionTask(0),
            Controller(str_controller),
            CanSendTo(set_can_send_to) {}
         /* members */
         CPiPuckEntity* Entity;
         UInt32 StepsUntilReturnToConstructionTask;
         std::string Controller;
         std::set<std::string> CanSendTo;
      };

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

      UInt32 m_unStepsUntilShadeCell;

      std::vector<bool> m_vecCells;

      std::array<UInt32, 2> m_arrLayout;

      std::map<std::string, SPiPuck> m_mapRobots;

      std::vector<CPiPuckEntity*> m_vecRobots;

   };


}

#endif

