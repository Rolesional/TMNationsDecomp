#pragma once

namespace tmnf::legacy {

// FUN_00401550 and FUN_004015C0.
template <typename Value>
void releaseIntrusive(Value* const value) {
  if (value != nullptr) {
    value->release();
  }
}

// FUN_00401560 and FUN_004015D0. The new reference is retained before the old one drops.
template <typename Value>
void assignIntrusive(Value*& destination, Value* const value) {
  if (value == destination) {
    return;
  }
  if (value != nullptr) {
    value->addRef();
  }
  releaseIntrusive(destination);
  destination = value;
}

} // namespace tmnf::legacy
