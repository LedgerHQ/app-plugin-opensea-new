import { nano_models, processTest, populateTransaction } from "../test.fixture";

const contractName = "SeaPort";
const contractAddr = "0xa0c68c638235ee32657e8f720a23cec1bfc77c77";
const testNetwork = "ethereum";

const testLabel = "polygon bridge"; // <= Name of the test
const testDirSuffix = testLabel.toLowerCase().replace(/\s+/g, '_');

// https://etherscan.io/tx/0x9a1dd3a01ebd3e6e50aa62ccf66ce4933e28d8a8446f35b1acc2c6b69f9d01bf
const inputData = "0x4faa8a2600000000000000000000000052a6bcebff5b28a79ccf91cecd4d4e598d8ee4bc";
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
