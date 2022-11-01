import { nano_models, processTest, populateTransaction } from "../test.fixture";

const contractName = "SeaPort";
const contractAddr = "0x00000000006c3852cbef3e08e8df289169ede581";
const testNetwork = "optimism";

const testLabel = "crosschain optimism fulfillBasicOrder buy op 1nft"; // <= Name of the test
const testDirSuffix = testLabel.toLowerCase().replace(/\s+/g, '_');

// https://optimistic.etherscan.io/tx/0xe85ef1fcf56f2f0ce30b04e7de144a89dd056fabb8c7a61bb025393949491b12
const inputData = "0xfb0f3ee10000000000000000000000000000000000000000000000000000000000000020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000022a392c68f60000000000000000000000000004643d7104e588cc5f0933bd94c23b85c88d007400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000e12103a2f6a41a812c1f58bd02afac93f3e43e6c00000000000000000000000000000000000000000000000000000000000008bd00000000000000000000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000006354e63d00000000000000000000000000000000000000000000000000000000637c64aa0000000000000000000000000000000000000000000000000000000000000000360c6ebe0000000000000000000000000000000000000000b6d0ad4da97cf9d30000007b02230091a7ed01230072f7006a004d60a8d4e71d599b8104250f00000000007b02230091a7ed01230072f7006a004d60a8d4e71d599b8104250f00000000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000024000000000000000000000000000000000000000000000000000000000000002a000000000000000000000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000e35fa931a0000000000000000000000000000000a26b00c1f0df003000390027140000faa7190000000000000000000000000000000000000000000000000000000000000041425001a5f6dac378e7e3383d1219f0f2e604b64c17daab0d3c144f7ee4222f4b3e5737b869699752a02d5fa65b4be9994d907d19317b0ee27d188774ee2acb941c00000000000000000000000000000000000000000000000000000000000000360c6ebe";
const models = [
    {
        name: 'nanos',
        steps: 9
    },
    // {
    // 	name: 'nanox',
    // 	steps: 0
    // },
]

// populate unsignedTx from genericTx and get network chain id
const unsignedTx = populateTransaction(contractAddr, inputData, testNetwork);
// Process tests for each nano models
models.forEach((model) => {
    const nano_model = nano_models.find((nano_model) => nano_model.name === model.name)
    processTest(nano_model, model.steps, contractName, testLabel, testDirSuffix, unsignedTx, testNetwork)
})