//============================================================================
// Name        : Lab4
// Author      : Branden Lee
// Date        : 5/30/2018
// Description : Encryption and Compression
//============================================================================

/**
 * Objectives:
 * 1. No raw pointers
 * 	In previous labs I used raw pointers frequently. My labs assume the compiler is "dumb"
 * 	and does not perform optimizations very well, such as RVO (Return Value Optimization).
 * 	In this lab I have attempted to use smart pointers and references. I still do not return objects
 * 	by value even though any modern compiler should optimize this very well.
 */

#include <string>
#include <fstream>
#include <iostream>		// std::cout
#include <sstream>
#include <iomanip>
#include <vector>		// std::vector
#include <stack>
#include <queue>		// std::priority_queue
#include <cstring>
#include <memory>		// std::unique_ptr
using namespace std;

/** Buffer size for reading in files for parsing */
#define PRIORITY_QUEUE_PARSER_BUFFER_SIZE 256

/**
 @class FileHandler
 simply reads or writes the decoded or encoded message to a file\n
 */
class FileHandler {
public:
	FileHandler() {
	}
	~FileHandler() {
	}
	/** takes a file stream by reference and opens a file.\n
	 * the reason we do not return the string of the entire ASCII file
	 * is because we want to stream and not waste memory
	 @pre None
	 @post None
	 @param string File name to open
	 @return True on file open successful and false in not
	 */
	bool readStream(string fileName, ifstream& fileStream);
	bool writeStream(string fileName, ofstream& fileStream);
	bool writeString(string fileName, string stringValue);
	bool close(ifstream& fileStream);
};

/**
 @class HashTable
 A simple hash table \n
 */
template<class K, class T>
class HashTable {
private:
	vector<pair<K, T>*> table;
	unsigned int insertAttempts;
public:
	HashTable();
	HashTable(unsigned int size);
	~HashTable() {
	}
	bool insert(K key, T val);
	T at(K key);
	T atIndex(unsigned int index);
	unsigned int hash(K key);
	unsigned int hash(unsigned int key);
	unsigned int size();
	bool resize(unsigned int key);
};

/**
 @class CharacterFrequencyNode
 similar to std::pair<unsigned int, unsigned int> except has that operator< overloaded \n
 first = frequency \n
 second = ascii character code \n
 */
class CharacterFrequencyNode {
public:
	unsigned int first, second;
	CharacterFrequencyNode(unsigned int first_, unsigned int second_);
	bool operator<(const CharacterFrequencyNode &lhs,
			const CharacterFrequencyNode &rhs) {
		return lhs.first < rhs.first;
	}
};

/**
 * @class CharacterPriorityQueueCompare
 * Function object for performing comparisons. Uses operator< on type T. \n
 */
template<class T>
class CharacterPriorityQueueCompare {
	bool operator()(const T &lhs, const T &rhs) const {
		return lhs < rhs;
	}
};

/**
 * The priority queue type is long so we make it into an alias
 * using is used rather than typedef because of this forum:
 * https://stackoverflow.com/questions/10747810/what-is-the-difference-between-typedef-and-using-in-c11
 */
using priorityQueueType = std::priority_queue<CharacterFrequencyNode, vector<CharacterFrequencyNode>, CharacterPriorityQueueCompare<CharacterFrequencyNode>>;

/**
 * @class CharacterPriorityQueue
 * document -> HashTable -> priority_queue -> set -> binary string \n
 */
class CharacterPriorityQueue {
private:
	/** HashTable<character code, character frequency> */
	HashTable<unsigned int, unsigned int*> characterFrequencyTable;
	unique_ptr<priorityQueueType> priorityQueue;
public:
	CharacterPriorityQueue() {
	}
	~CharacterPriorityQueue() {
	}
	/**
	 * parses document into a character frequency table \n
	 * Afterward, call <pre>buildPriorityQueue()</pre> to build the priority queue
	 * @return true on parse success, false on failure
	 */
	bool fileStreamIn(istream& streamIn);
	bool bufferHandle(string& streamBuffer);
	/**
	 * builds the priority queue from the character frequency table
	 * @return true on build success, false on failure
	 */
	bool buildPriorityQueue();
	/**
	 * @return priority queue reference
	 */
	reference_wrapper<priorityQueueType> getPriorityQueue();
};

class CharacterPriorityQueueTreeNode {
private:
	CharacterPriorityQueueTreeNode *left, *right;
	unsigned int characterCode;
	bool leafFlag;
public:
	CharacterPriorityQueueTreeNode();
};

/**
 @class CharacterPriorityQueueTree
 Tables to convert character codes to binary strings and back \n
 */
class CharacterPriorityQueueTree {
private:
	CharacterPriorityQueueTreeNode characterPriorityQueueTreeNodeParent;
public:
	CharacterPriorityQueueTree();
	/*
	 * @param priorityQueuePtr observing pointer
	 */
	void buildTree(priorityQueueType* priorityQueuePtr);
	unique_ptr<HashTable<unsigned int, string>> toCharacterToBinaryTable();
};

/**
 @class CharacterToBinaryTable
 Tables to convert character codes to binary strings and back \n
 */
class CharacterToBinaryTable {
private:
	/** HashTable<character code, character binary string> */
	unique_ptr<HashTable<unsigned int, string>> characterCodeToBinaryStringTablePtr;
	/** HashTable<character binary string, character code> */
	unique_ptr<HashTable<string, unsigned int>> binaryStringToCharacterCodeTablePtr;
	CharacterPriorityQueueTreeNode characterPriorityQueueTreeNodeParent;
public:
	/**
	 * to keep this class "slim", we will build our table elsewhere and move the pointer
	 * ownership into this class.
	 */
	CharacterToBinaryTable(unique_ptr<HashTable<unsigned int, string>> tablePtr) :
			characterCodeToBinaryStringTablePtr(move(tablePtr)) {
	}
	CharacterToBinaryTable(unique_ptr<HashTable<string, unsigned int>> tablePtr) :
			binaryStringToCharacterCodeTablePtr(move(tablePtr)) {
	}
	/**
	 * build the binaryStringToCharacterCodeTable from the characterCodeToBinaryStringTable
	 */
	void buildBinaryStringToCharacterCodeTable();
	/**
	 * build the characterCodeToBinaryStringTable from the binaryStringToCharacterCodeTable
	 */
	void buildCharacterCodeToBinaryStringTable();
	/**
	 * Primary encoding method
	 * @param characterCode Character character code
	 * @return Character Binary String
	 */
	string characterCodeToBinaryString(unsigned int characterCode);
	/**
	 * Primary decoding method
	 * @param characterBinaryString Character Binary String
	 * @return character code
	 */
	unsigned int binaryStringToCharacterCode(string characterBinaryString);
};

class Product {
private:
	string name;
	double price;
public:
	Product(string name_, double price_);
	~Product() {
	}
	string getName();
	double getPrice();
	string toString();
};

class ProductTable {
private:
	Code39CharTable code39CharTable;
	HashTable<string, Product*> code39ItemToProductTable;
public:
	ProductTable();
	~ProductTable() {
	}
	/*
	 * @param key The code39 Binary String of first 5 characters of product name
	 * @param val pointer to the product
	 */
	bool insert(string key, Product* val);
	/*
	 * Generates the key automatically
	 * @param val pointer to the product
	 */
	bool insert(Product* val);
	/*
	 * WARNING: could throw an exception
	 */
	Product* at(string key);
	string toString();

};

class Cart {
private:
	vector<Product*> productList;
	unsigned int cartNumber;
	double priceTotal;
public:
	Cart();
	Cart(unsigned int cartNumber_);
	~Cart() {
	}
	void insert(Product* productPtr);
	void calculatePriceTotal();
	string toString();
};

class CartList {
private:
	vector<Cart*> cartList;
public:
	CartList() {
	}
	~CartList() {
	}
	void insert(Cart* cardPtr);
	string toString();
};

class CartLane {
private:
	XMLNode* nodeXMLCarts;
	CartList* cartListObject;
	ProductTable* productTableObject;
	unsigned int indexStart;
	unsigned int indexStop;
public:
	CartLane() {
		nodeXMLCarts = NULL;
		cartListObject = NULL;
		productTableObject = NULL;
		indexStart = indexStop = 0;
	}
	~CartLane() {
	}
	void init(XMLNode*& nodeXMLCarts_, CartList& cartListObject_,
			ProductTable& productTableObject_);
	void range(unsigned int indexStart_, unsigned int indexStop_);
	bool process();
};

/**
 @class Code39Item
 Converts a Code 39 Item to the a product \n
 */
class Code39Item {
private:
	Code39CharTable * code39CharTable;
	string binaryString, codeString;
	queue<int> intQueue;
public:
	Code39Item(Code39CharTable* code39CharTablePtr);
	~Code39Item() {
	}
	void setBinaryString(string binaryString_);
	void setCodeString(string codeString_);
	bool toCodeString(string& code39CharItem);
	bool toBinaryString(string& code39CharItem);
};

/*
 * FileHandler Implementation
 */
bool FileHandler::close(ifstream& fileStream) {
	fileStream.close();
	return true;
}

bool FileHandler::readStream(string fileName, ifstream& fileStream) {
	fileStream.open(fileName, ios::binary);
	if (fileStream.is_open()) {
		return true;
	}
	return false;
}

bool FileHandler::writeStream(string fileName, ofstream& fileStream) {
	fileStream.open(fileName);
	if (fileStream.is_open()) {
		return true;
	}
	return false;
}

bool FileHandler::writeString(string fileName, string stringValue) {
	ofstream fileStream;
	fileStream.open(fileName);
	if (fileStream.is_open()) {
		fileStream << stringValue;
		fileStream.close();
		return true;
	}
	return false;
}

/*
 * HashTable Implementation
 */
template<class K, class T>
HashTable<K, T>::HashTable() {
	table.resize(100);
	insertAttempts = 10;
}

template<class K, class T>
bool HashTable<K, T>::insert(K key, T val) {
	unsigned int attempts = insertAttempts;
	K keyOriginal = key;
	unsigned int keyInt = hash(key);
	bool flag = false;
	for (; attempts > 0; attempts--) {
		if (table[keyInt] == nullptr) {
			table[keyInt] = new pair<K, T>(keyOriginal, val);
			flag = true;
			break;
		}
		keyInt = hash(keyInt);
	}
	return flag;
}
template<class K, class T>
T HashTable<K, T>::at(K key) {
	unsigned int attempts = insertAttempts;
	K keyOriginal = key;
	unsigned int keyInt = hash(key);
	T ret { };
	for (; attempts > 0; attempts--) {
		if (table[keyInt] != nullptr && table[keyInt]->first == keyOriginal) {
			ret = table[keyInt]->second;
			break;
		}
		keyInt = hash(keyInt);
	}
	if (attempts == 0) {
		throw out_of_range("");
	}
	return ret;
}
template<class K, class T>
T HashTable<K, T>::atIndex(unsigned int index) {
	pair<K, T>* temp = table[index];
	if (temp != nullptr) {
		return temp->second;
	} else {
		throw invalid_argument("");
	}
}
template<class K, class T>
unsigned int HashTable<K, T>::hash(K key) {
	unsigned int hash = 1315423911;
	unsigned int i = 0;
	while (key[i++]) {
		hash ^= ((hash << 5) + key[i] + (hash >> 2));
	}
	return (hash & 0x7FFFFFFF) % table.size();
}
template<class K, class T>
unsigned int HashTable<K, T>::hash(unsigned int key) {
	// stackoverflow.com/questions/664014/what-integer-hash-function-are-good-that-accepts-an-integer-hash-key
	key = ((key >> 16) ^ key) * 0x45d9f3b;
	key = ((key >> 16) ^ key) * 0x45d9f3b;
	key = (key >> 16) ^ key;
	return key % table.size();
}
template<class K, class T>
unsigned int HashTable<K, T>::size() {
	return table.size();
}

template<class K, class T>
bool HashTable<K, T>::resize(unsigned int key) {
	bool flag = false;
	if (key > table.size()) {
		table.resize(key);
		flag = true;
	}
	return flag;
}

/*
 * CharacterFrequencyNode Implementation
 */
CharacterFrequencyNode::CharacterFrequencyNode(unsigned int first_,
		unsigned int second_) {
	first = first_;
	second = second_;
}
/*
 * CharacterPriorityQueue Implementation
 */
bool CharacterPriorityQueue::fileStreamIn(istream& streamIn) {
	/* Parsing Steps:
	 * 1. create document node. If stack is empty then document node is the parent.
	 * 2. grab first <tag> and add push on stack.
	 *    future nodes will be a child of top of the stack
	 * 3. grab child node and push it to the stack.
	 * 4. value between <child></child> is added to the node on top of the stack
	 * 5. if </tag> found then it is popped off the stack
	 * */
	unsigned int fileSize, filePos, bufferSize, mode;
	string streamBuffer;
	stack<string> documentStack;
	bufferSize = PRIORITY_QUEUE_PARSER_BUFFER_SIZE;
	fileSize = filePos = mode = 0;
	streamBuffer = "";
	char bufferInChar[PRIORITY_QUEUE_PARSER_BUFFER_SIZE];
	streamIn.seekg(0, ios::end); // set the pointer to the end
	fileSize = (unsigned int) streamIn.tellg(); // get the length of the file
	streamIn.seekg(0, ios::beg); // set the pointer to the beginning
	while (filePos < fileSize) {
		streamIn.seekg(filePos, ios::beg); // seek new position
		if (filePos + bufferSize > fileSize) {
			bufferSize = fileSize - filePos;
			memset(bufferInChar, 0, sizeof(bufferInChar)); // zero out buffer
		}
		memset(bufferInChar, 0, sizeof(bufferInChar)); // zero out buffer
		streamIn.read(bufferInChar, bufferSize);
		streamBuffer.append(bufferInChar, bufferSize);
		bufferHandle(streamBuffer);
		// advance buffer
		filePos += bufferSize;
	}
	// handle the remaining buffer
	bufferHandle(streamBuffer);
	return true;
}

bool CharacterPriorityQueue::bufferHandle(string& streamBuffer) {
	/* characters are placed into a hash table
	 * hashTable[CharacterCode] = CharacterFrequency
	 */
	unsigned int i, n, characterCode;
	bool found;
	unsigned int* characterFrequencyPtr;
	n = streamBuffer.size();
	for (i = 0; i < n; i++) {
		found = false;
		characterCode =
				static_cast<unsigned int>(static_cast<int>(streamBuffer[i]));
		try {
			// if there was no exception thrown, the character was found
			characterFrequencyPtr = characterFrequencyTable.at(characterCode);
			found = true;
		} catch (...) {
			// nothing
		}
		if (found) {
			// character exists. increment frequency
			*characterFrequencyPtr++;
		} else {
			characterFrequencyTable.insert(characterCode, new unsigned int(1));
		}
	}
	// there actually is no point to the return, but maybe there will be in the future
	return true;
}

reference_wrapper<priorityQueueType> CharacterPriorityQueue::getPriorityQueue() {
	return ref(priorityQueue.get());
}

/*
 * CharacterPriorityQueueTreeNode Implementation
 */
CharacterPriorityQueueTreeNode::CharacterPriorityQueueTreeNode() {
	left = right = nullptr;
	characterCode = 0;
	leafFlag = false;
}

/*
 * CharacterToBinaryTable Implementation
 */

void CharacterToBinaryTable::buildBinaryStringToCharacterCodeTable() {
	unsigned int i, n, n1;
	/* extend to lower case characters */
	n = ((int) 'Z') + 1;
	for (i = (int) 'A'; i < n; i++) {
		if (charIntToCode39IntTable[i]
				&& (n1 = charIntToCode39IntTable[i]) > 0) {
			charIntToCode39IntTable[(unsigned int) tolower(char(i))] =
					charIntToCode39IntTable[i];
		}
	}
	/* 2^9 since the longest Code 39 Binary is 9 bits */
	Code39IntToCharTable.resize(512);
	// build a binary int to char map
	n = (unsigned int) charIntToCode39IntTable.size();
	for (i = 0; i < n; i++) {
		if (charIntToCode39IntTable[i]
				&& (n1 = charIntToCode39IntTable[i]) > 0) {
			// we are worried of bits above 9
			try {
				Code39IntToCharTable[n1] = char(i);
			} catch (...) {
				// nothing
			}
		}
	}
}

string CharacterToBinaryTable::characterCodeToBinaryString(
		unsigned int characterCode) {
	bool returnValue = false;
	try {
		charOut = Code39IntToCharTable[intIn];
		returnValue = true;
	} catch (...) {
		// nothing
	}
	return returnValue;
}

unsigned int CharacterToBinaryTable::binaryStringToCharacterCode(
		string characterBinaryString) {
	bool returnValue = false;
	try {
		intOut = charIntToCode39IntTable[(unsigned int) charIn];
		returnValue = true;
	} catch (...) {
		// nothing
	}
	return returnValue;
}

/*
 * Product Implementation
 */
Product::Product(string name_, double price_) {
	name = name_;
	price = price_;
}
string Product::getName() {
	return name;
}
double Product::getPrice() {
	return price;
}
string Product::toString() {
	// I'm using string stream so I don't need to reinvent console spacing.
	stringstream str;
	str << left << setw(20) << name << " " << fixed << setprecision(2) << price;
	return str.str();
}

/*
 * ProductTable Implementation
 */
ProductTable::ProductTable() {
	code39ItemToProductTable.resize(1700);
}
bool ProductTable::insert(string key, Product* val) {
	return code39ItemToProductTable.insert(key, val);
}
bool ProductTable::insert(Product* val) {
	bool returnValue = false;
	Code39Item code39Item(&code39CharTable);
	code39Item.setCodeString(val->getName().substr(0, 5));
	string code39BinaryString;
	if (code39Item.toBinaryString(code39BinaryString)) {
		/*cout << val->getName().substr(0, 5) << " = " << code39BinaryString
		 << endl;*/
		returnValue = code39ItemToProductTable.insert(code39BinaryString, val);
	}
	return returnValue;
}
Product* ProductTable::at(string key) {
	return code39ItemToProductTable.at(key);
}
string ProductTable::toString() {
	string str = "";
	str.append(" Product Table \n---------------\n");
	stringstream headString, endString;
	headString << left << setw(20) << "Product Name" << " Price" << endl;
	str.append(headString.str());
	unsigned int i, n;
	n = code39ItemToProductTable.size();
	for (i = 0; i < n; i++) {
		try {
			str.append(code39ItemToProductTable.atIndex(i)->toString()).append(
					"\n");
		} catch (...) {

		}
	}
	return str;
}
/*
 * Cart Implementation
 */
Cart::Cart() {
	cartNumber = 0;
	priceTotal = 0;
}
Cart::Cart(unsigned int cartNumber_) {
	cartNumber = cartNumber_;
	priceTotal = 0;
}
void Cart::insert(Product* productPtr) {
	productList.push_back(productPtr);
}
void Cart::calculatePriceTotal() {
	unsigned int i, n;
	n = (unsigned int) productList.size();
	priceTotal = 0;
	for (i = 0; i < n; i++) {
		priceTotal += productList.at(i)->getPrice();
	}
}
string Cart::toString() {
	string str = "";
	stringstream headString, endString;
	calculatePriceTotal();
	str.append("Cart #").append(to_string(cartNumber)).append("\n");
	headString << left << setw(20) << "Product Name" << " Price" << endl;
	str.append(headString.str());
	unsigned int i, n;
	n = (unsigned int) productList.size();
	try {
		for (i = 0; i < n; i++) {
			str.append(productList[i]->toString()).append("\n");
		}
	} catch (...) {
		//nothing
	}
	endString << left << setfill('-') << setw(30) << "" << endl << setfill(' ')
			<< setw(21) << "Total Price" << fixed << setprecision(2)
			<< priceTotal << endl << setfill('-') << setw(30) << "";
	str.append(endString.str());
	return str;
}
/*
 * CartList Implementation
 */
void CartList::insert(Cart* cartPtr) {
	cartList.push_back(cartPtr);
}
string CartList::toString() {
	string str = "";
	str.append(" Cart List \n-----------\n");
	unsigned int i, n;
	n = cartList.size();
	try {
		for (i = 0; i < n; i++) {
			str.append(cartList[i]->toString()).append("\n\n");
		}
	} catch (...) {
		//nothing
	}
	return str;
}
/*
 * CartLane Implementation
 */
void CartLane::init(XMLNode*& nodeXMLCarts_, CartList& cartListObject_,
		ProductTable& productTableObject_) {
	nodeXMLCarts = nodeXMLCarts_;
	cartListObject = &cartListObject_;
	productTableObject = &productTableObject_;
}
void CartLane::range(unsigned int indexStart_, unsigned int indexStop_) {
	indexStart = indexStart_;
	indexStop = indexStop_;
}
bool CartLane::process() {
	unsigned int i, n, j, n1, cartNumber;
	XMLNode *nodeCart, *nodeItem;
	Cart* cartPtr = NULL;
	// Assume all children are carts
	n = indexStop;
	cout << indexStart << " to " << n << endl;
	for (i = indexStart; i < n; i++) {
		nodeCart = nodeXMLCarts->getChild(i);
		if (nodeCart != nullptr) {
			/* extract the cart number
			 * stoi could throw exceptions
			 */
			try {
				cartNumber = (unsigned int) stoi(
						nodeCart->getName().substr(4,
								nodeCart->getName().length()));
			} catch (...) {
				cartNumber = 0;
			}
			cartPtr = new Cart(cartNumber);
			// Assume all children are items
			n1 = nodeCart->childrenSize();
			for (j = 0; j < n1; j++) {
				nodeItem = nodeCart->getChild(j);
				if (nodeItem != nullptr) {
					try {
						cartPtr->insert(
								productTableObject->at(nodeItem->getValue()));
					} catch (...) {
						//nothing
					}
				}
			}
		}
		mutexGlobal.lock();
		cartListObject->insert(cartPtr);
		mutexGlobal.unlock();
	}
	return true;
}
/*
 * Code39Item Implementation
 */
Code39Item::Code39Item(Code39CharTable* code39CharTablePtr) {
	code39CharTable = code39CharTablePtr;
}

void Code39Item::setBinaryString(string binaryString_) {
	unsigned int i, n;
	n = binaryString_.size();
	// must have at least one code39 char
	if (n / 9 > 0 && n % 9 == 0) {
		for (i = 0; i < n; i++) {
			bitset < 9 > bits(binaryString_.substr(i * 9, 9));
			intQueue.push((unsigned int) bits.to_ulong());
		}
	}
}
void Code39Item::setCodeString(string codeString_) {
	unsigned int i, n, temp;
	n = codeString_.size();
	for (i = 0; i < n; i++) {
		code39CharTable->charToInt(codeString_[i], temp);
		intQueue.push(temp);
	}
}

bool Code39Item::toCodeString(string& code39CharItem) {
	bool returnValue = false;
	if (!intQueue.empty()) {
		char temp[2];
		temp[1] = 0;
		code39CharItem = "";
		while (!intQueue.empty()) {
			code39CharTable->intToChar(intQueue.front(), temp[0]);
			code39CharItem.append(temp);
			intQueue.pop();
		}
		returnValue = true;
	}
	return returnValue;
}
bool Code39Item::toBinaryString(string& code39CharItem) {
	bool returnValue = false;
	if (!intQueue.empty()) {
		code39CharItem = "";
		while (!intQueue.empty()) {
			bitset < 9 > bits(intQueue.front());
			code39CharItem.append(bits.to_string());
			intQueue.pop();
		}
		returnValue = true;
	}
	return returnValue;
}

/*
 * main & interface
 * Rules For Encoding:
 * - Read through file and increment character code frequency in a hash table based on contents
 * - Generate frequency table with priority queue
 * - Create binary tree from priority queue
 * - Encrypt the input file as an encrypted binary file
 *
 * Rules For Decoding
 * - Decrypt the encrypted binary file using the existing priority queue
 *
 */
int main() {
	FileHandler fh;
	Parser parser;
	CharacterPriorityQueue characterPriorityQueue;
	string fileNameOriginal, fileNameEncrypt, fileNameDecrypt;
	ifstream fileStreamIn;
	ofstream fileStreamOut;
	future<bool> parseProductsXMLFuture, parseCartsXMLFuture;
	bool flag = false;
	/* input/output files are here */
	fileNameOriginal = "Speech.txt";
	fileNameEncrypt = "encrypt.data";
	fileNameDecrypt = "decrypt.txt";
	// parse XML file streams into an XML document node
	XMLNode ProductsXML, CartsXML;
	ProductTable productTable;
	CartList cartList;
	if (!fh.readStream(fileNameOriginal, fileStreamIn)) {
		cout << "Could not read either of the input files." << endl;
	} else {
		cout << "Parsing XML files..." << endl;
		/* we pass file streams instead of a string to this method
		 * because we want to stream the data and decode it as we read.
		 * This way very large files won't lag or crash the program.
		 */
		characterPriorityQueue.fileStreamIn(fileStreamIn);
		CharacterPriorityQueueTree characterPriorityQueueTree();
		CharacterToBinaryTable characterToBinaryTable(
				characterPriorityQueueTree.toCharacterToBinaryTable());
		if (parseProductsXMLFuture.get()) {
			cout
					<< "Successfully parsed Product List XML File to Product List Nodes."
					<< endl;
			// create the product table from the product list XML
			if (parser.productListXMLNodetoObject(ProductsXML, productTable)) {
				//fh.writeString("productList.txt", productTable.toString());
				cout
						<< "Successfully parsed Product List XML Nodes into hash table."
						<< endl;
				flag = true;
			} else {
				cout
						<< "Failed to parse Product List XML Nodes into hash table."
						<< endl;
			}
		}
	}
	// process each cart from the XML file referencing each product from the product table
	if (flag && parseCartsXMLFuture.get()) {
		cout << "Successfully parsed Cart List XML file into cart list nodes."
				<< endl << "Processing Carts..." << endl;
		// close the XML files
		fh.close(fileStreamInProducts);
		fh.close(fileStreamInCarts);
		//cout << "XML Files successfully parsed!" << endl;
		if (parser.cartListXMLNodetoObject(CartsXML, cartList, productTable)) {
			if (fh.writeString(fileNameCartsList, cartList.toString())) {
				cout << "Carts list written to \"" << fileNameCartsList << "\""
						<< endl;
			}
		}
	}
	cout << "Enter any key to exit..." << endl;
	string temp;
	getline(cin, temp);
	return 0;
}
