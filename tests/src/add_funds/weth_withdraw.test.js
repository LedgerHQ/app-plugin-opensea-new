import { nano_models, processTest, populateTransaction } from "../test.fixture";

const contractName = "SeaPort";
const contractAddr = "0xc02aaa39b223fe8d0a0e5c4f27ead9083c756cc2";
const testNetwork = "ethereum";

const testLabel = "weth withdraw"; // <= Name of the test
const testDirSuffix = testLabel.toLowerCase().replace(/\s+/g, '_');

// https://etherscan.io/tx/0xe45b63e95da69ef5a446cbf1585b815e13bdb4a225676d58c64235d913752e89
const inputData = "0x2e1a7d4d000000000000000000000000000000000000000000000000016345785d8a0000";
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
