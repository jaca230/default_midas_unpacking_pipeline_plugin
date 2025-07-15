#include "stages/unpacking/base_midas_unpacker_stage.h"
#include <stdexcept>
#include <typeinfo>

ClassImp(BaseMidasUnpackerStage)

BaseMidasUnpackerStage::BaseMidasUnpackerStage() = default;
BaseMidasUnpackerStage::~BaseMidasUnpackerStage() = default;

void BaseMidasUnpackerStage::SetInput(const InputBundle& input) {
    if (!input.has<TMEvent>("TMEvent")) {
        throw std::runtime_error("BaseMidasUnpackerStage::SetInput - InputBundle missing TMEvent");
    }
    TMEvent& event = input.getRef<TMEvent>("TMEvent");
    SetCurrentEvent(event);
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
