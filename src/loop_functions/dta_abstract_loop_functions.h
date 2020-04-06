#ifndef DTA_ABSTRACT_LOOP_FUNCTIONS_H
#define DTA_ABSTRACT_LOOP_FUNCTIONS_H

namespace argos {
   class CPiPuckEntity;
}

#include <argos3/core/simulator/loop_functions.h>

#include <array>
#include <fstream>
#include <map>
#include <string>
#include <set>
#include <vector>

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
         UInt32 PreviousX;
         UInt32 PreviousY;
      };

   public:

      CDTAAbstractLoopFunctions();

      virtual ~CDTAAbstractLoopFunctions() {}

      virtual void Init(TConfigurationNode& t_tree);

      virtual CColor GetFloorColor(const CVector2& c_position);

      virtual void Reset();

      virtual void Destroy();

      virtual void PostStep();

   private:
      /* loop function configuration */
      std::array<UInt32, 2> m_arrGridLayout;
      Real m_fMeanForagingDurationInitial;
      Real m_fMeanForagingDurationGradient;
      UInt32 m_unConstructionLimit;
      enum class EShadingDistribution {
         UNIFORM, POISSON
      } m_eShadingDistribution;
      
      /* loop function output */
      std::string m_strOutputFilename;
      std::ostream* m_pcOutput;

      /* loop function state */
      std::vector<bool> m_vecCells;
      std::map<std::string, SPiPuck> m_mapRobots;
      std::vector<UInt32> m_vecConstructionEvents;
   };

}

#endif

