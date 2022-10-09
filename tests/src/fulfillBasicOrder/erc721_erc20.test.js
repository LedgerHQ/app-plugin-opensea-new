import { nano_models, processTest, populateTransaction } from "../test.fixture";

const contractName = "SeaPort";
const contractAddr = "0x00000000006c3852cbef3e08e8df289169ede581";
const testNetwork = "ethereum";

const testLabel = "fullfilBasicOrder erc721_erc20"; // <= Name of the test
const testDirSuffix = testLabel.toLowerCase().replace(/\s+/g, '_');

// https://etherscan.io/tx/0x4cf93750f453ceefcd19f02273b300a156fa6e383f46ab728010db1a4cbe7973
const inputData = "0xfb0f3ee100000000000000000000000000000000000000000000000000000000000000200000000000000000000000008d04a8c79ceb0889bdd12acdf3fa9d207ed3ff63000000000000000000000000000000000000000000000000000000000000067e0000000000000000000000000000000000000000000000000000000000000001000000000000000000000000ca74ebb0c97b5b7f66d3a5dd96d64612b82efd0d000000000000000000000000004c00500000ad104d7dbd00e3ae0a5c00560c00000000000000000000000000c02aaa39b223fe8d0a0e5c4f27ead9083c756cc20000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000283edea298a20000000000000000000000000000000000000000000000000000000000000000001200000000000000000000000000000000000000000000000000000000633d8b3c0000000000000000000000000000000000000000000000000000000063403c20300000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000653ec4cbe957eee5a9528197872caa020000007b02230091a7ed01230072f7006a004d60a8d4e71d599b8104250f00000000007b02230091a7ed01230072f7006a004d60a8d4e71d599b8104250f00000000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000024000000000000000000000000000000000000000000000000000000000000002a000000000000000000000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000101925daa3740000000000000000000000000000000a26b00c1f0df003000390027140000faa7190000000000000000000000000000000000000000000000000000000000000040f08f1ebc52fde0cdee337daa467fe8dd13b32a437ea4dd8b505ba49f0a86dc5f025f5ba6f1fc1ca57ea87bb780c7ee8b7475a94e6a23559dc486ca274cb25a52360c6ebe";

const models = [
  {
    name: 'nanos',
    steps: 5
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
