import { nano_models, processTest, populateTransaction } from "../test.fixture";

const contractName = "L1StandardBridge"
const contractAddr = "0x99c9fc46f92e8a1c0dec1b1747d010903e884be1";
const testNetwork = "ethereum";
const txValue = '0.071142839733849676';

const testLabel = "optimism bridge"; // <= Name of the test
const testDirSuffix = testLabel.toLowerCase().replace(/\s+/g, '_');

// https://etherscan.io/tx/0x56cfc41d8bf79c04c77a2dcb6dbb7ccbcad9951931bb4a0b45c3f22be1c5198a
const inputData = "0xb1a1a8820000000000000000000000000000000000000000000000000000000000030d4000000000000000000000000000000000000000000000000000000000000000400000000000000000000000000000000000000000000000000000000000000000";
const models = [
    {
        name: 'nanos',
        steps: 6
    },
    // {
    // 	name: 'nanox',
    // 	steps: 0
    // },
]

// populate unsignedTx from genericTx and get network chain id
const unsignedTx = populateTransaction(contractAddr, inputData, testNetwork, txValue);
// Process tests for each nano models
models.forEach((model) => {
    const nano_model = nano_models.find((nano_model) => nano_model.name === model.name)
    processTest(nano_model, model.steps, contractName, testLabel, testDirSuffix, unsignedTx, testNetwork)
})
