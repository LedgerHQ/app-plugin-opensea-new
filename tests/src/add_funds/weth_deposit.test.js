import { nano_models, processTest, populateTransaction } from "../test.fixture";

const contractName = "SeaPort";
const contractAddr = "0x00000000006c3852cbef3e08e8df289169ede581";
const testNetwork = "ethereum";

const testLabel = "weth deposit"; // <= Name of the test
const testDirSuffix = testLabel.toLowerCase().replace(/\s+/g, '_');

// https://etherscan.io/tx/0x21bb0673a5ef3b3bdb32b0ef7f97f49008abe9dd73ed003b8898b1a037b2fb15 
const inputData = "0xd0e30db0";
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
