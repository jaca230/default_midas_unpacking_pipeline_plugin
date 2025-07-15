#ifndef ANALYSIS_PIPELINE_STAGES_BASEMIDASUNPACKERSTAGE_H
#define ANALYSIS_PIPELINE_STAGES_BASEMIDASUNPACKERSTAGE_H

#include "stages/input/base_input_stage.h"
#include "midas.h"
#include "midasio.h"
#include <any>

class BaseMidasUnpackerStage : public BaseInputStage {
public:
    BaseMidasUnpackerStage();
    ~BaseMidasUnpackerStage() override;

    // Receives externally injected input (expects TMEvent inside std::any)
    void SetInput(const InputBundle& input) override;

    // Run unpacking on the most recent input
    void Process() final override;

protected:
    // Called internally after casting input to TMEvent
    void SetCurrentEvent(TMEvent& event);

    const TMEvent* current_event_ = nullptr;

    // Subclasses implement MIDAS unpacking logic here
    virtual void ProcessMidasEvent(TMEvent& event) = 0;

    ClassDefOverride(BaseMidasUnpackerStage, 1);
};

#endif // ANALYSIS_PIPELINE_STAGES_BASEMIDASUNPACKERSTAGE_H
