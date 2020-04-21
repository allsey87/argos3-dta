#ifndef DTA_LOOP_FUNCTIONS_H
#define DTA_LOOP_FUNCTIONS_H

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

   class CDTALoopFunctions : public CLoopFunctions {

   public:

      struct SPiPuck {
         SPiPuck(const std::string& str_controller) :
            Entity(nullptr),
            StepsUntilReturnToConstructionTask(0),
            Controller(str_controller) {}
         /* members */
         CPiPuckEntity* Entity;
         UInt32 StepsUntilReturnToConstructionTask;
         std::string Controller;
         UInt32 PreviousX;
         UInt32 PreviousY;
      };

   public:

      CDTALoopFunctions();

      virtual ~CDTALoopFunctions() {}

      virtual void Init(TConfigurationNode& t_tree);

      virtual CColor GetFloorColor(const CVector2& c_position);

      virtual void Reset();

      virtual void Destroy();

      virtual void PostStep();

   private:

      bool IsOnGrid(const CVector2& c_position);

      std::pair<UInt32, UInt32> GetGridCoordinatesFor(const CVector2& c_position);

      void ShadeCellUniform();

      void ShadeCellBiased();

      void UnshadeCellUniform();

      void UnshadeCellBiased();

   private:
      /* loop function configuration */
      std::array<UInt32, 2> m_arrGridLayout;
      Real m_fMeanForagingDurationInitial;
      Real m_fMeanForagingDurationGradient;
      Real m_fInitialShadingRatio;
      bool m_bEnableForaging;
      UInt32 m_unConstructionLimit;
      enum class EShadingDistribution {
         UNIFORM, BIASED
      } m_eShadingDistribution;
      UInt32 m_unShadingBias = 5u;
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

