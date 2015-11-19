#pragma once

#include "ISource.h"
#include "commandArgUtil.h" // for intargvector
#include "CUDAPageLockedMemAllocator.h"
#include "minibatchiterator.h"
#include "chunkevalsource.h"

namespace Microsoft { namespace MSR { namespace CNTK {

    template<class ElemType>
    class HTKMLFSource : public ISource
    {
        unique_ptr<msra::dbn::minibatchiterator> m_mbiter;
        unique_ptr<msra::dbn::minibatchsource> m_frameSource;
        unique_ptr<msra::dbn::FileEvalSource> m_fileEvalSource;
        unique_ptr<msra::dbn::latticesource> m_lattices;
        map<wstring, msra::lattices::lattice::htkmlfwordsequence> m_latticeMap;

        std::vector<InputDefinition> m_inputs;

        vector<bool> m_sentenceEnd;
        bool m_frameMode;
        vector<size_t> m_processedFrame;
        size_t m_mbNumTimeSteps;                // number of time steps  to fill/filled (note: for frame randomization, this the #frames, and not 1 as later reported)
        vector<size_t> m_numFramesToProcess;    // [seq index] number of frames available (left to return) in each parallel sequence
        vector<size_t> m_switchFrame;           /// TODO: something like the position where a new sequence starts; still supported?
        vector<size_t> m_numValidFrames;        // [seq index] valid #frames in each parallel sequence. Frames (s, t) with t >= m_numValidFrames[s] are NoInput.
        vector<size_t> m_extraSeqsPerMB;
        size_t m_extraNumSeqs;
        bool m_noData;
        bool m_trainOrTest; // if false, in file writing mode

        typedef std::string LabelType;
        typedef unsigned int LabelIdType;

        bool m_partialMinibatch; // allow partial minibatches?

        std::vector<std::shared_ptr<ElemType>> m_featuresBufferMultiUtt;
        std::vector<size_t> m_featuresBufferAllocatedMultiUtt;
        std::vector<std::shared_ptr<ElemType>> m_labelsBufferMultiUtt;
        std::vector<size_t> m_labelsBufferAllocatedMultiUtt;
        std::vector<size_t> m_featuresStartIndexMultiUtt;
        std::vector<size_t> m_labelsStartIndexMultiUtt;

        unique_ptr<CUDAPageLockedMemAllocator> m_cudaAllocator;

        //for lattice uids and phoneboundaries
        std::vector<shared_ptr<const msra::dbn::latticesource::latticepair>>  m_latticeBufferMultiUtt;
        std::vector<std::vector<size_t>> m_labelsIDBufferMultiUtt;
        std::vector<std::vector<size_t>> m_phoneboundaryIDBufferMultiUtt;
        std::vector<shared_ptr<const msra::dbn::latticesource::latticepair>>  m_extraLatticeBufferMultiUtt;
        std::vector<std::vector<size_t>> m_extraLabelsIDBufferMultiUtt;
        std::vector<std::vector<size_t>> m_extraPhoneboundaryIDBufferMultiUtt;

        //hmm 
        msra::asr::simplesenonehmm m_hset;

        std::map<std::wstring, size_t> m_featureNameToIdMap;
        std::map<std::wstring, size_t> m_labelNameToIdMap;
        std::map<std::wstring, size_t> m_nameToTypeMap;
        std::map<std::wstring, size_t> m_featureNameToDimMap;
        std::map<std::wstring, size_t> m_labelNameToDimMap;

        // for writing outputs to files (standard single input/output network) - deprecate eventually
        bool m_checkDictionaryKeys;
        bool m_convertLabelsToTargets;
        std::vector <bool> m_convertLabelsToTargetsMultiIO;
        std::vector<std::vector<std::wstring>> m_inputFilesMultiIO;

        size_t m_inputFileIndex;
        std::vector<size_t> m_featDims;
        std::vector<size_t> m_labelDims;

        std::vector<std::vector<std::vector<ElemType>>>m_labelToTargetMapMultiIO;

        int m_verbosity;

        bool GetMinibatch4SEToTrainOrTest(std::vector<shared_ptr<const msra::dbn::latticesource::latticepair>> & latticeinput, vector<size_t> &uids, vector<size_t> &boundaries, std::vector<size_t> &extrauttmap);


        void StartMinibatchLoopToTrainOrTest(size_t mbSize, size_t epoch, size_t subsetNum, size_t numSubsets, size_t requestedEpochSamples = requestDataSize);

        enum InputOutputTypes
        {
            real,
            category,
        };

    public:
        /// by default it is false
        /// if true, reader will set to ((int) MinibatchPackingFlags::None) for time positions that are orignally correspond to ((int) MinibatchPackingFlags::SequenceStart)
        /// set to true so that a current minibatch can uses state activities from the previous minibatch. 
        /// default will have truncated BPTT, which only does BPTT inside a minibatch
        bool mIgnoreSentenceBeginTag;
        // TODO: this ^^ does not seem to belong here.

        virtual void Init(const ConfigParameters& config);
        virtual void Destroy() { delete this; }
        virtual ~HTKMLFSource() { }

        virtual void StartMinibatchLoop(size_t mbSize, size_t epoch, size_t requestedEpochSamples = requestDataSize)
        {
            return StartDistributedMinibatchLoop(mbSize, epoch, 0, 1, requestedEpochSamples);
        }

        virtual bool SupportsDistributedMBRead() const
        {
            return m_frameSource && m_frameSource->supportsbatchsubsetting();
        }

        virtual void StartDistributedMinibatchLoop(size_t mbSize, size_t epoch, size_t subsetNum, size_t numSubsets, size_t requestedEpochSamples = requestDataSize);

        virtual bool GetHmmData(msra::asr::simplesenonehmm * hmm);

        virtual Timeline& getTimeline() override;
        virtual std::map<size_t, Sequence> getSequenceById(sequenceId id) override;
        virtual std::vector<InputDefinition> getInputs() override;

        private:
        void AdvanceIteratorToNextDataPortion();
};

}
}
}
