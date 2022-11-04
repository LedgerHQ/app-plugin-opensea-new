import { nano_models, processTest, populateTransaction } from "../test.fixture";

const contractName = "Seaport";
const contractAddr = "0x00000000006c3852cbef3e08e8df289169ede581";
const testNetwork = "ethereum";

const testLabel = "increment counter"; // <= Name of the test
const testDirSuffix = testLabel.toLowerCase().replace(/\s+/g, '_');

// https://etherscan.io/tx/0xaabf2fd347e6de758cea660007a3d5ccaedfeb35009ddcfb3da4b21c2c181859
const inputData = "0x5b34b966";
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
