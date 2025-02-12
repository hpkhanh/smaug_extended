#ifndef _CORE_OPERATOR_H_
#define _CORE_OPERATOR_H_

#include <string>
#include <vector>
#include <map>
#include <boost/format.hpp>

#include "backend.h"
#include "backend_config.h"
#include "smaug/core/typedefs.h"
#include "smaug/core/tensor.h"
#include "smaug/core/tensor_utils.h"
#include "smaug/core/types.pb.h"
#include "smaug/operators/common.h"
#include "smaug/core/network_config.h"

#define REGISTER_SPECIAL_OP(Operator, Backend)                                 \
    template <>                                                                \
    void Operator<Backend>::run();

namespace smaug {

class Workspace;

constexpr const char* kLayerFormat = "%-40s %-25s %=15d\n";

/**
 * Operator is the base class for all graph operators supported by SMAUG.
 */
class Operator {
   public:
    Operator(const std::string& _name, OpType _opType, Workspace* _workspace)
            : name(_name), opType(_opType), workspace(_workspace),
              numPendingInputs(-1) {}
    virtual ~Operator() {}

    virtual void tile() {};

    /**
     * Executes the Operator.
     *
     * All Operator subclasses must override this method.
     */
    virtual void run() = 0;

    /**
     * Returns true if the parameters/tensors of this operator are all valid.
     */
    virtual bool validate() {
        return opType != OpType::UnknownOp;
    }

    /**
     * For tests: creates all input and output tensors for this operator.
     *
     * When running a network, all tensor shapes are specified in the
     * network topology proto, and the network builder will automatically create
     * them for you. In unit tests, this is a convenience method to avoid
     * needing to create TensorShape protos.
     *
     * This should only be called once the Operator is fully initialized with
     * all required parameters. It is responsible for creating only the tensors
     * it "owns". All operators "own" their output tensors, but not necessarily
     * all of their input tensors.  For example, a convolution operator "owns"
     * its weight tensors, but not the input activations (which are the output
     * of a previous Operator).
     *
     * Note that "ownership" of a Tensor does not mean the Operator holds a
     * std::unique_ptr to the Tensor; it simply means it is solely responsible
     * for constructing and allocating memory for it.
     */
    virtual void createAllTensors() {}

    /**
     * Returns true if the Operator is dead.
     *
     * By default, an Operator is dead if any input is dead, but operators can
     * override this behavior if they are expected to operate on dead Tensors.
     */
    virtual bool isDead();

    /**
     * Return a list of Tensors whose values that are parameterizable.
     *
     * Parameterizable Tensors often include "weights", but typically do not
     * include outputs of other Tensors.
     */
    virtual std::vector<TensorBase*> getParameterizableInputs() { return {}; }

    /** This returns the number of parameterizable weights in the operator. */
    virtual int getNumParameters() const { return 0; }
    virtual bool isSamplingSupported() const { return false; }
    virtual void setSamplingInfo(const SamplingInfo& sampling) {}

    void printSummary(std::ostream& out) const;
    void setInput(TensorBase* op, int index) { inputs[index] = op; }
    void setOutput(TensorBase* op, int index) { outputs[index] = op; }

    /**
     * Set the number of input tensors that this operator is waiting on. When
     * this value drops to zero, this operator is ready to be scheduled.
     * */
    void setNumPendingInputs(int num) { numPendingInputs = num; }
    int getNumPendingInputs() const { return numPendingInputs; }
    void decrNumPendingInputs() { numPendingInputs--; }
    const std::string& getName() const { return name; }
    Vertex getVertex() const { return vertex; }
    void setVertex(Vertex v) { vertex = v; }
    void setBackEnd(BackEndName_t be) { backEnd = be; }
    void setNumCores(int cores) { numCores = cores; }
    void setMemSize(unsigned long long size) { memSize = size; }
    void setNumPEs(int num) { numPEs = num; }
    void setNumMaccsPerPE(int numMaccs) { numMaccsPerPE = numMaccs; }

    BackEndName_t getBackEnd() { return backEnd; }
    int getNumCores() { return numCores; }
    unsigned long long getMemSize() { return memSize; }
    int getNumPEs() { return numPEs; }
    int getNumMaccsPerPE() { return numMaccsPerPE; }
    OpType getOpType() const { return opType; }
    Workspace* getWorkspace() { return workspace; }

    Tensor* getInput(int index) const {
        return dynamic_cast<Tensor*>(inputs.at(index));
    }
    const std::vector<TensorBase*>& getInputs() const { return inputs; }

    Tensor* getOutput(int index) const {
        return dynamic_cast<Tensor*>(outputs.at(index));
    }
    const std::vector<TensorBase*>& getOutputs() const { return outputs; }

    void setInputsMemType(MemoryType type) { inputsMemType = type; }
    void setWeightsMemType(MemoryType type) { weightsMemType = type; }
    void setOutputsMemType(MemoryType type) { outputsMemType = type; }
    MemoryType getInputsMemType() const { return inputsMemType; }
    MemoryType getWeightsMemType() const { return weightsMemType; }
    MemoryType getOutputsMemType() const { return outputsMemType; }

   protected:
    /** An ordered list of input tensors consumed by this operator.
     *
     * Operators may assign semantic meaning to specific input tensors at
     * specific positions in this array, e.g. index 0 is the input data Tensor
     * and index 1 is the weight Tensor.
     */
    std::vector<TensorBase*> inputs;

    /** An ordered list of output tensors produced by this operator.
     *
     * Like input tensors, operators can assign semantic meaning to tensors at
     * specific positions here.
     */
    std::vector<TensorBase*> outputs;

    std::string name;
    OpType opType;
    BackEndName_t backEnd;
    int numCores; 
    unsigned long long memSize;
    int numPEs;
    int numMaccsPerPE;

    /** The BGL Vertex corresponding to this Operator. */
    Vertex vertex;
    Workspace* workspace;
    /** The number of tensors that this operator is waiting on before it can be
     * scheduled. */
    int numPendingInputs;
    /** The memory interface over which input activations are expected to arrive. */
    MemoryType inputsMemType;
    /** The memory interface over which weights are expected to arrive. */
    MemoryType weightsMemType;
    /** The memory interface over which outputs are expected to be delivered. */
    MemoryType outputsMemType;
};

}  // namespace smaug

#endif
