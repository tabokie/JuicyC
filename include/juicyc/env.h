#ifndef JUICYC_ENV_H_
#define JUICYC_ENV_H_

#include "system.h"

namespace juicyc {

struct Env {
 public:
 	Env() = default;
 	Env(const Env&) = delete;
 	Env(Env&& rhs)
 	    : input_system_(std::move(rhs.input_system_)),
 	      output_system_(std::move(rhs.output_system_)) {}
 	void set_input_system(InputSystem* in) { input_system_.reset(in); }
 	void set_output_system(OutputSystem* out) { output_system_.reset(out); }
 	InputSystem* input_system() { return input_system_.get(); }
 	OutputSystem* output_system() { return output_system_.get(); }
 protected:
 	std::unique_ptr<InputSystem> input_system_ = nullptr;
 	std::unique_ptr<OutputSystem> output_system_ = nullptr;
};

}  // namespace juicyc

#endif  // JUICYC_ENV_H_