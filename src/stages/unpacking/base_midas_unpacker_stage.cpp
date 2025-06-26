#include "stages/unpacking/base_midas_unpacker_stage.h"
#include <stdexcept>
#include <typeinfo>

ClassImp(BaseMidasUnpackerStage)

BaseMidasUnpackerStage::BaseMidasUnpackerStage() = default;
BaseMidasUnpackerStage::~BaseMidasUnpackerStage() = default;

void BaseMidasUnpackerStage::SetInput(std::any input) {
    // Try to cast std::any to TMEvent reference
    try {
        TMEvent& event = std::any_cast<std::reference_wrapper<TMEvent>>(input).get();
        SetCurrentEvent(event);
    } catch (const std::bad_any_cast& e) {
        throw std::runtime_error("BaseMidasUnpackerStage::SetInput - input is not a TMEvent reference");
    }
}

void BaseMidasUnpackerStage::SetCurrentEvent(TMEvent& event) {
    current_event_ = &event;
}

void BaseMidasUnpackerStage::Process() {
    if (!current_event_) {
        throw std::runtime_error("BaseMidasUnpackerStage: current_event_ not set");
    }
    ProcessMidasEvent(*const_cast<TMEvent*>(current_event_));
}
