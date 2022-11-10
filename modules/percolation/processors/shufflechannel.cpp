/*********************************************************************
 *  Author  : Tino Weinkauf
 *  Init    : Friday, April 05, 2019 - 16:46:52
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <percolation/processors/shufflechannel.h>

#include <random>

namespace inviwo
{


// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ShuffleChannel::processorInfo_
{
    "org.inviwo.ShuffleChannel",      // Class identifier
    "Shuffle Channel",                // Display name
    "Percolation",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};

const ProcessorInfo ShuffleChannel::getProcessorInfo() const
{
    return processorInfo_;
}


ShuffleChannel::ShuffleChannel()
    :Processor()
    ,connInData("InData")
    ,connOutData("OutData")
    ,propChannel(connInData, "Channel", "Channel")
    //,propGenerateNewChannel("GenerateNewChannel", "New Channel")
    ,propGroupRandom("GroupRandom", "Randomness")
    ,propFixedSeed("FixedSeed", "Fixed Seed")
    ,propSeed("Seed", "Seed", 0, 0, std::numeric_limits<unsigned int>::max(), 1)
{
    addPort(connInData);
    addPort(connOutData);

    addProperty(propChannel);
    //addProperty(propGenerateNewChannel);

    addProperty(propGroupRandom);
    propSeed.setSemantics(PropertySemantics::Text);
    propGroupRandom.addProperties(propFixedSeed, propSeed);

    propFixedSeed.onChange([&]() { propSeed.setReadOnly(!propFixedSeed.get()); });
}


namespace
{
    template <typename T, ind N>
    Channel* MakeAndShuffleTheBuffer(const DataChannel<T, N>& InputChannel, unsigned int Seed)
    {
        //Create Buffer and copy input channel into it
        BufferChannel<T, N>* ShuffledBuffer = new BufferChannel<T, N>(InputChannel.size(), "Shuffled " + InputChannel.getName(), InputChannel.getGridPrimitiveType());
        for (ind element = 0; element < InputChannel.size(); element++)
        {
            InputChannel.fill(ShuffledBuffer->template get<std::array<T, N>>(element), element);
        }
        ShuffledBuffer->copyMetaDataFrom(InputChannel);

        //Shuffle it.
        ///@todo: This does not work for anything N > 1. The vector does not know about the number of components.
        std::vector<T>& RawData = ShuffledBuffer->data();
        std::shuffle(RawData.begin(), RawData.end(), std::default_random_engine(Seed));

        return ShuffledBuffer;
    }
}

void ShuffleChannel::process()
{
    //Get input data
    const auto InData = connInData.getData();
    if (!InData) return;
    const auto InChannel = propChannel.getCurrentChannel();
    if (!InChannel) return;

    //Get output data
    auto OutData = std::make_shared<DataSet>(*InData.get());

    //Seed
    if (!propFixedSeed.get())
    {
        //Generate new seed
        propSeed.set((unsigned int)rand());
    }
    //else: Use seed as given by user: propSeed.get(); see below

    //Shuffle!
    Channel* pShuffledData = InChannel->dispatch<Channel*, dispatching::filter::All, 1, 4>(
        [&](const auto* pInputChannel)
        {
            return MakeAndShuffleTheBuffer(*pInputChannel, propSeed.get());
        });

    //Set output data
    OutData->addChannel(pShuffledData);
    connOutData.setData(OutData);
}

} // namespace

