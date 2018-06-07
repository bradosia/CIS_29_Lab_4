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
 * 2. Independent Compressed Binary
 * 	The lab instructions assume that files will be encoded then promptly decoded with the information
 * 	in memory. This project will try to make the encoded binary files independent of the program
 * 	instance. The program may encode a file, close, reopen and decode the file without any
 * 	extra information.
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
	/** HashTable<unsigned int, string> */
	shared_ptr<HashTable<unsigned int, string>> characterToBinaryTable;
public:
	CharacterPriorityQueueTree();
	/**
	 * @param priority queue reference
	 * @return true on build success, false on build failure
	 */
	bool buildTree(priorityQueueType& priorityQueue);
	bool buildBinaryTable();
	shared_ptr<HashTable<unsigned int, string>> getCharacterToBinaryTable();
};

/**
 @class CharacterToBinaryTable
 Tables to convert character codes to binary strings and back \n
 */
class CharacterToBinaryTable {
private:
	/** HashTable<character code, character binary string> */
	shared_ptr<HashTable<unsigned int, string>> characterCodeToBinaryStringTablePtr;
	/** HashTable<character binary string, character code> */
	shared_ptr<HashTable<string, unsigned int>> binaryStringToCharacterCodeTablePtr;
	CharacterPriorityQueueTreeNode characterPriorityQueueTreeNodeParent;
public:
	CharacterToBinaryTable() {

	}
	/**
	 * to keep this class "slim", we will build our table elsewhere and move the pointer
	 * ownership into this class.
	 */
	void set(shared_ptr<HashTable<unsigned int, string>> tablePtr) {
		characterCodeToBinaryStringTablePtr = tablePtr;
	}
	void set(shared_ptr<HashTable<string, unsigned int>> tablePtr) {
		binaryStringToCharacterCodeTablePtr = tablePtr;
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

class Compressor {
private:
	shared_ptr<CharacterToBinaryTable> characterToBinaryTable;
public:
	void set(shared_ptr<CharacterToBinaryTable> tablePtr);
	bool encode(istream& streamIn, ostream& streamOut);
	bool decode(istream& streamIn, ostream& streamOut);
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
	CharacterPriorityQueue characterPriorityQueue;
	CharacterPriorityQueueTree characterPriorityQueueTree;
	shared_ptr<CharacterToBinaryTable> characterToBinaryTablePtr = make_shared<
			CharacterToBinaryTable>();
	shared_ptr<Compressor> compressorPtr = make_shared<Compressor>();
	string fileNameOriginal, fileNameEncrypt, fileNameDecrypt;
	ifstream fileStreamIn;
	ofstream fileStreamOut;
	/**
	 * anti deep if nesting flag
	 * I could also use an exception model to stop deep nesting, but
	 * exceptions will hurt performance more. Instead we will use a flag to
	 * reset a deep nest back to the highest level context in the function
	 */
	bool flag = false;
	/* input/output files are here */
	fileNameOriginal = "Speech.txt";
	fileNameEncrypt = "encrypt.data";
	fileNameDecrypt = "decrypt.txt";
	cout << "Opening the input file." << endl;
	if (!fh.readStream(fileNameOriginal, fileStreamIn)
			|| !fh.writeStream(fileNameEncrypt, fileStreamOut)) {
		/*
		 * We check a false value first so we can quickly reference messages
		 * in the code. Otherwise we would have to scroll a bit down to find if and corresponding
		 * false message. This is just convenience.
		 */
		cout << "[Error] Could not open either the input file \""
				<< fileNameOriginal << "\" or the input file \""
				<< fileNameEncrypt << "\"." << endl;
	} else {
		cout << "Parsing input input stream as a character frequency table."
				<< endl;
		/* we pass a file stream instead of a reading the whole file
		 * into memory to reduce memory usage.
		 */
		if (!characterPriorityQueue.fileStreamIn(fileStreamIn)) {
			cout
					<< "[Error] Could not parse the input stream as a character frequency table."
					<< endl;
		} else {
			cout << "Building the priority queue." << endl;
			if (!characterPriorityQueue.buildPriorityQueue()) {
				cout << "[Error] Could not build the priority queue." << endl;
			} else {
				flag = true;
			}
		}
	}
	// deep if nesting reset
	if (flag) {
		flag = false;
		cout << "Building the priority queue tree." << endl;
		if (!characterPriorityQueueTree.buildTree(
				characterPriorityQueue.getPriorityQueue())) {
			cout << "[Error] Could not build the priority queue tree." << endl;
		} else {
			cout << "Building the binary string table." << endl;
			if (!characterPriorityQueueTree.buildBinaryTable()) {
				cout << "[Error] Could not build the binary string table."
						<< endl;
			} else {
				characterToBinaryTablePtr->set(
						characterPriorityQueueTree.getCharacterToBinaryTable());
				// only need for decoding
				// characterToBinaryTable.buildBinaryStringToCharacterCodeTable();
				compressorPtr->set(characterToBinaryTablePtr);
				flag = true;
			}
		}
	}
	if (flag) {
		flag = false;
		cout << "Encoding the input file." << endl;
		if (!compressorPtr->encode(fileStreamIn,)) {
			cout << "[Error] Could not build the binary string table." << endl;
		} else {
			characterToBinaryTablePtr->set(
					characterPriorityQueueTree.getCharacterToBinaryTable());
			// only need for decoding
			// characterToBinaryTable.buildBinaryStringToCharacterCodeTable();
			compressorPtr->set(characterToBinaryTablePtr);
			flag = true;
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
