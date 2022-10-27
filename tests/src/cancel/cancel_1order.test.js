import { nano_models, processTest, populateTransaction } from "../test.fixture";

const contractName = "SeaPort";
const contractAddr = "0x00000000006c3852cbef3e08e8df289169ede581";
const testNetwork = "ethereum";

const testLabel = "cancel 1order"; // <= Name of the test
const testDirSuffix = testLabel.toLowerCase().replace(/\s+/g, '_');

// https://etherscan.io/tx/0x612e498e03cccc8ea23f59027112370a0474f6366a8a1b58fc1d96dcf4d7b71a
const inputData = "0xfd9f1e10000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000000000020000000000000000000000000af75696b63745470e74b752c38f8413095d076a7000000000000000000000000004c00500000ad104d7dbd00e3ae0a5c00560c0000000000000000000000000000000000000000000000000000000000000001600000000000000000000000000000000000000000000000000000000000000220000000000000000000000000000000000000000000000000000000000000000200000000000000000000000000000000000000000000000000000000634412c900000000000000000000000000000000000000000000000000000000636cff590000000000000000000000000000000000000000000000000000000000000000360c6ebe00000000000000000000000000000000000000009782ef63f5ac1bfb0000007b02230091a7ed01230072f7006a004d60a8d4e71d599b8104250f00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000020000000000000000000000008b4616926705fb61e9c4eeac07cd946a5d4b0760000000000000000000000000000000000000000000000000000000000000257c0000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000030000000000000000000000000000000000000000000000000000000000000001000000000000000000000000a0b86991c6218b36c1d19d4a2e9eb0ce3606eb48000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000564718e000000000000000000000000000000000000000000000000000000000564718e0000000000000000000000000af75696b63745470e74b752c38f8413095d076a70000000000000000000000000000000000000000000000000000000000000001000000000000000000000000a0b86991c6218b36c1d19d4a2e9eb0ce3606eb48000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000023c346000000000000000000000000000000000000000000000000000000000023c34600000000000000000000000000000a26b00c1f0df003000390027140000faa7190000000000000000000000000000000000000000000000000000000000000001000000000000000000000000a0b86991c6218b36c1d19d4a2e9eb0ce3606eb4800000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000e4e1c00000000000000000000000000000000000000000000000000000000000e4e1c0000000000000000000000000147b8869655bc09f226955cc676ff78efe240ca8360c6ebe";
const models = [
    {
        name: 'nanos',
        steps: 4
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
