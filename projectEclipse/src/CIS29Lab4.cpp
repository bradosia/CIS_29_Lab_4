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
#include <functional>	// std::hash
#include <bitset>		// std::bitset
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
	bool close(ofstream& fileStream);
};

/**
 @class HashTable
 A simple hash table \n
 */
template<class K, class T>
class HashTable {
private:
	vector<unique_ptr<pair<K, T>>> table;
	unsigned int insertAttempts;
	hash<K> hashT;
	hash<size_t> hashSize;
public:
	HashTable() :
			table(100), insertAttempts(10) {
	}
	HashTable(size_t size) :
			table(size), insertAttempts(10) {
	}
	~HashTable() {
	}
	bool insert(K key, T val);
	bool at(K key, T& val);
	bool atIndex(size_t index, K& key, T& val);
	size_t size();
	bool resize(size_t key);
};

/**
 @class CharacterFrequencyNode
 similar to std::pair<unsigned int, unsigned int> except has that operator< overloaded \n
 first = frequency \n
 second = ascii character code \n
 */
class CharacterFrequencyNode {
public:
	unsigned int frequency, characterCode;
	CharacterFrequencyNode(unsigned int frequency_,
			unsigned int characterCode_);
	// "this" is already the left hand side
	bool operator<(const CharacterFrequencyNode &rhs) {
		return frequency < rhs.frequency;
	}
	bool operator>(const CharacterFrequencyNode &rhs) {
		return frequency > rhs.frequency;
	}
};

/**
 @class CharacterPriorityQueueTreeNode
 */
class CharacterPriorityQueueTreeNode {
private:
public:
	unsigned int frequency;
	CharacterPriorityQueueTreeNode() :
			frequency(0) {

	}
	virtual ~CharacterPriorityQueueTreeNode() {

	}
	unsigned int getFrequency();
	virtual bool isLeaf() {
		return false;
	}
	virtual bool isBranch() {
		return false;
	}
};

/**
 @class CharacterPriorityQueueTreeLeaf
 */
class CharacterPriorityQueueTreeLeaf: public CharacterPriorityQueueTreeNode {
private:
	shared_ptr<CharacterFrequencyNode> characterFrequencyNode;
public:
	CharacterPriorityQueueTreeLeaf(
			shared_ptr<CharacterFrequencyNode> characterNode) :
			characterFrequencyNode(characterNode) {
		frequency = characterNode->frequency;
	}
	~CharacterPriorityQueueTreeLeaf() {

	}
	bool isLeaf();
	bool isBranch();
	shared_ptr<CharacterFrequencyNode> getCharacterNode();
};

/**
 @class CharacterPriorityQueueTreeBranch
 */
class CharacterPriorityQueueTreeBranch: public CharacterPriorityQueueTreeNode {
private:
	shared_ptr<CharacterPriorityQueueTreeNode> left, right;
public:
	CharacterPriorityQueueTreeBranch(
			shared_ptr<CharacterPriorityQueueTreeNode> left_,
			shared_ptr<CharacterPriorityQueueTreeNode> right_) :
			left(left_), right(right_) {
		frequency = left_->getFrequency() + right_->getFrequency();
	}
	~CharacterPriorityQueueTreeBranch() {

	}
	bool isLeaf();
	bool isBranch();
	shared_ptr<CharacterPriorityQueueTreeNode> getLeft();
	shared_ptr<CharacterPriorityQueueTreeNode> getRight();
};

/**
 * @class CharacterPriorityQueueCompare
 * Function object for performing comparisons. Uses operator< on type T. \n
 */
template<class T>
class CharacterPriorityQueueCompare {
public:
	bool operator()(const T &lhs, const T &rhs) const {
		return lhs->getFrequency() > rhs->getFrequency();
	}
};

/**
 * The priority queue type is long so we make it into an alias
 * using is used rather than typedef because of this forum:
 * https://stackoverflow.com/questions/10747810/what-is-the-difference-between-typedef-and-using-in-c11
 */
using priorityQueueType = std::priority_queue<shared_ptr<CharacterPriorityQueueTreeNode>, vector<shared_ptr<CharacterPriorityQueueTreeNode>>, CharacterPriorityQueueCompare<shared_ptr<CharacterPriorityQueueTreeNode>>>;

/**
 * @class CharacterPriorityQueueF
 * document -> HashTable -> priority_queue -> set -> binary string \n
 */
class CharacterPriorityQueue {
private:
	/** HashTable<character code, character frequency>
	 * character frequency is a shared pointer to make it easy to increment */
	HashTable<unsigned int, shared_ptr<unsigned int>> characterFrequencyTable;
	priorityQueueType priorityQueue;
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

/**
 @class CharacterPriorityQueueTree
 Tables to convert character codes to binary strings and back \n
 */
class CharacterPriorityQueueTree {
private:
	shared_ptr<CharacterPriorityQueueTreeNode> characterPriorityQueueTreePtr;
	/** HashTable<unsigned int, string> */
	shared_ptr<HashTable<unsigned int, string>> characterToBinaryTablePtr;
public:
	CharacterPriorityQueueTree() {

	}
	~CharacterPriorityQueueTree() {

	}
	/**
	 * build tree will edit the priority queue so we must pass by value
	 * to prevent tampering
	 * @param priority queue copy
	 * @return true on build success, false on build failure
	 */
	bool buildTree(priorityQueueType priorityQueue);
	bool buildBinaryTable();
	void buildBinaryTableEncode(shared_ptr<CharacterPriorityQueueTreeNode> node,
			string binaryString);
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
	void set(shared_ptr<HashTable<unsigned int, string>> tablePtr);
	void set(shared_ptr<HashTable<string, unsigned int>> tablePtr);
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
	bool characterCodeToBinaryString(unsigned int characterCode,
			string& binaryString);
	/**
	 * Primary decoding method
	 * @param characterBinaryString Character Binary String
	 * @return character code
	 */
	bool binaryStringToCharacterCode(string binaryString,
			unsigned int characterCode);
};

class Compressor {
private:
	shared_ptr<CharacterToBinaryTable> characterToBinaryTablePtr;
public:
	void set(shared_ptr<CharacterToBinaryTable> tablePtr);
	bool encode(istream& streamIn, ostream& streamOut);
	bool encodeBufferHandle(string& streamBufferIn, string& streamBufferOut,
			ostream& streamOut);
	bool decode(istream& streamIn, ostream& streamOut);
};

/*
 * FileHandler Implementation
 */
bool FileHandler::readStream(string fileName, ifstream& fileStream) {
	fileStream.open(fileName, ios::binary);
	if (fileStream.is_open()) {
		return true;
	}
	throw 1;
	return false;
}

bool FileHandler::writeStream(string fileName, ofstream& fileStream) {
	fileStream.open(fileName, ios::binary);
	if (fileStream.is_open()) {
		return true;
	}
	throw 2;
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

bool FileHandler::close(ifstream& fileStream) {
	try {
		fileStream.close();
	} catch (...) {
		throw 7;
		return false;
	}
	return true;
}

bool FileHandler::close(ofstream& fileStream) {
	try {
		fileStream.close();
	} catch (...) {
		throw 8;
		return false;
	}
	return true;
}

/*
 * HashTable Implementation
 */
template<class K, class T>
bool HashTable<K, T>::insert(K key, T val) {
	// will overwrite a key that already exists
	bool returnValue = false;
	unsigned int attempts = insertAttempts;
	K keyOriginal = key;
	size_t size = table.size();
	size_t keyInt = hashT(key) % size;
	for (; attempts > 0; attempts--) {
		// bucket does not exist or if it does it will overwrite if the keys match
		if (!table[keyInt]
				|| (table[keyInt] && table[keyInt]->first == keyOriginal)) {
			table[keyInt] = make_unique<pair<K, T>>(keyOriginal, val);
			returnValue = true;
			break;
		}
		keyInt = hashSize(keyInt) % size;
	}
	return returnValue;
}
template<class K, class T>
bool HashTable<K, T>::at(K key, T& val) {
	/* 2018-6-7 revision
	 * no longer throws exception because it creates a large overhead when
	 * attempting to find lots of bad keys
	 */
	bool returnValue = false;
	unsigned int attempts = insertAttempts;
	K keyOriginal = key;
	size_t size = table.size();
	size_t keyInt = hashT(key) % size;
	for (; attempts > 0; attempts--) {
		// bucket exists and matches key
		if (table[keyInt] && table[keyInt]->first == keyOriginal) {
			val = table[keyInt]->second;
			returnValue = true;
			break;
		}
		keyInt = hashSize(keyInt) % size;
	}
	return returnValue;
}
template<class K, class T>
bool HashTable<K, T>::atIndex(size_t index, K& key, T& val) {
	/* 2018-6-7 revision
	 * no longer throws exception because it creates a large overhead when
	 * trying to traverse the entire hash table
	 */
	bool returnValue = false;
	if (table[index]) {
		key = table[index]->first;
		val = table[index]->second;
		returnValue = true;
	}
	return returnValue;
}

template<class K, class T>
size_t HashTable<K, T>::size() {
	return table.size();
}

template<class K, class T>
bool HashTable<K, T>::resize(size_t key) {
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
CharacterFrequencyNode::CharacterFrequencyNode(unsigned int frequency_,
		unsigned int characterCode_) {
	frequency = frequency_;
	characterCode = characterCode_;
}

/*
 * CharacterPriorityQueueTreeNode Implementation
 */
unsigned int CharacterPriorityQueueTreeNode::getFrequency() {
	return frequency;
}
/*
 * CharacterPriorityQueueTreeLeaf Implementation
 */
bool CharacterPriorityQueueTreeLeaf::isLeaf() {
	return true;
}
bool CharacterPriorityQueueTreeLeaf::isBranch() {
	return false;
}
shared_ptr<CharacterFrequencyNode> CharacterPriorityQueueTreeLeaf::getCharacterNode() {
	return characterFrequencyNode;
}

/*
 * CharacterPriorityQueueTreeBranch Implementation
 */
bool CharacterPriorityQueueTreeBranch::isLeaf() {
	return false;
}
bool CharacterPriorityQueueTreeBranch::isBranch() {
	return true;
}
shared_ptr<CharacterPriorityQueueTreeNode> CharacterPriorityQueueTreeBranch::getLeft() {
	return left;
}
shared_ptr<CharacterPriorityQueueTreeNode> CharacterPriorityQueueTreeBranch::getRight() {
	return right;
}
/*
 * CharacterPriorityQueue Implementation
 */
bool CharacterPriorityQueue::fileStreamIn(istream& streamIn) {
	/* Parsing Steps:
	 * 1. read in buffer
	 * 2. go through each character in buffer
	 *    add the character code as hash table key and value as the frequency
	 * */
	unsigned int fileSize, filePos, bufferSize, mode;
	string streamBuffer;
	stack<string> documentStack;
	bufferSize = PRIORITY_QUEUE_PARSER_BUFFER_SIZE;
	fileSize = filePos = mode = 0;
	streamBuffer = "";
	char bufferInChar[PRIORITY_QUEUE_PARSER_BUFFER_SIZE];
	streamIn.seekg(0, ios::end); // set the pointer to the end
	fileSize = static_cast<unsigned int>(streamIn.tellg()); // get the length of the file
	streamIn.seekg(0, ios::beg); // set the pointer to the beginning
	while (filePos < fileSize) {
		streamIn.seekg(filePos, ios::beg); // seek new position
		if (filePos + bufferSize > fileSize) {
			bufferSize = fileSize - filePos;
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
	size_t i, n;
	n = streamBuffer.size();
	unsigned int characterCode;
	shared_ptr<unsigned int> characterFrequencyPtr;
	for (i = 0; i < n; i++) {
		characterCode =
				static_cast<unsigned int>(static_cast<int>(streamBuffer[i]));
		if (characterFrequencyTable.at(characterCode, characterFrequencyPtr)) {
			// character exists. increment frequency
			(*characterFrequencyPtr)++;
		} else {
			characterFrequencyTable.insert(characterCode,
					make_shared<unsigned int>(1));
		}
	}
	streamBuffer = "";
	// there actually is no point to the return, but maybe there will be in the future
	return true;
}

bool CharacterPriorityQueue::buildPriorityQueue() {
	size_t i, n;
	n = characterFrequencyTable.size();
	unsigned int characterCode;
	shared_ptr<unsigned int> characterFrequencyPtr;
	for (i = 0; i < n; i++) {
		if (characterFrequencyTable.atIndex(i, characterCode,
				characterFrequencyPtr)) {
			priorityQueue.push(
					dynamic_pointer_cast<CharacterPriorityQueueTreeNode>(
							make_shared<CharacterPriorityQueueTreeLeaf>(
									make_shared<CharacterFrequencyNode>(
											*characterFrequencyPtr,
											characterCode))));
		}
	}
	return true;
}

reference_wrapper<priorityQueueType> CharacterPriorityQueue::getPriorityQueue() {
	return ref(priorityQueue);
}

/*
 * CharacterPriorityQueueTree Implementation
 */
bool CharacterPriorityQueueTree::buildTree(priorityQueueType priorityQueue) {
	if (priorityQueue.size() > 0) {
		shared_ptr<CharacterPriorityQueueTreeNode> left;
		shared_ptr<CharacterPriorityQueueTreeNode> right;
		shared_ptr<CharacterPriorityQueueTreeNode> branch;
		while (priorityQueue.size() > 1) {
			left = priorityQueue.top();
			priorityQueue.pop();
			right = priorityQueue.top();
			priorityQueue.pop();
			branch = dynamic_pointer_cast<CharacterPriorityQueueTreeNode>(
					make_shared<CharacterPriorityQueueTreeBranch>(left, right));
			priorityQueue.push(branch);
			if (dynamic_pointer_cast<CharacterPriorityQueueTreeLeaf>(left)) {
				cout << "left char="
						<< dynamic_pointer_cast<CharacterPriorityQueueTreeLeaf>(
								left)->getCharacterNode()->characterCode;
			}
			if (dynamic_pointer_cast<CharacterPriorityQueueTreeLeaf>(right)) {
				cout << " right char="
						<< dynamic_pointer_cast<CharacterPriorityQueueTreeLeaf>(
								right)->getCharacterNode()->characterCode;
			}
			cout << " left freq=" << left->getFrequency() << " right freq="
					<< right->getFrequency() << " branch="
					<< branch->getFrequency() << endl;

		}
		characterPriorityQueueTreePtr = priorityQueue.top();
	}
	return true;
}
bool CharacterPriorityQueueTree::buildBinaryTable() {
	if (!characterToBinaryTablePtr) {
		characterToBinaryTablePtr =
				make_shared<HashTable<unsigned int, string>>(255);
	}
	if (characterPriorityQueueTreePtr) {
		buildBinaryTableEncode(characterPriorityQueueTreePtr, "");
	}
	size_t i, n;
	n = characterToBinaryTablePtr->size();
	unsigned int characterCode;
	string binaryString;
	for (i = 0; i < n; i++) {
		if (characterToBinaryTablePtr->atIndex(i, characterCode,
				binaryString)) {
			cout << "-> " << characterCode << " " << binaryString << endl;
		}
	}
	return true;
}
void CharacterPriorityQueueTree::buildBinaryTableEncode(
		shared_ptr<CharacterPriorityQueueTreeNode> node, string binaryString) {
	if (node->isBranch()) {
		if (dynamic_pointer_cast<CharacterPriorityQueueTreeBranch>(node)) {
			buildBinaryTableEncode(
					dynamic_pointer_cast<CharacterPriorityQueueTreeBranch>(node)->getLeft(),
					binaryString.append("0"));
			buildBinaryTableEncode(
					dynamic_pointer_cast<CharacterPriorityQueueTreeBranch>(node)->getRight(),
					binaryString.append("1"));
		}
	} else {
		if (dynamic_pointer_cast<CharacterPriorityQueueTreeLeaf>(node)) {
			characterToBinaryTablePtr->insert(
					dynamic_pointer_cast<CharacterPriorityQueueTreeLeaf>(node)->getCharacterNode()->characterCode,
					binaryString);
		}
	}
}
shared_ptr<HashTable<unsigned int, string>> CharacterPriorityQueueTree::getCharacterToBinaryTable() {
	return characterToBinaryTablePtr;
}

/*
 * CharacterToBinaryTable Implementation
 */
void CharacterToBinaryTable::set(
		shared_ptr<HashTable<unsigned int, string>> tablePtr) {
	characterCodeToBinaryStringTablePtr = tablePtr;
}

void CharacterToBinaryTable::set(
		shared_ptr<HashTable<string, unsigned int>> tablePtr) {
	binaryStringToCharacterCodeTablePtr = tablePtr;
}

void CharacterToBinaryTable::buildBinaryStringToCharacterCodeTable() {
	/*
	 * builds HashTable<binary string, character code>
	 */
	if (!binaryStringToCharacterCodeTablePtr) {
		// we only expect around 255 unique characters
		binaryStringToCharacterCodeTablePtr = make_shared<
				HashTable<string, unsigned int>>(255);
	}
	size_t i, n;
	n = characterCodeToBinaryStringTablePtr->size();
	unsigned int characterCode;
	string binaryString;
	for (i = 0; i < n; i++) {
		if (characterCodeToBinaryStringTablePtr->atIndex(i, characterCode,
				binaryString)) {
			binaryStringToCharacterCodeTablePtr->insert(binaryString,
					characterCode);
		}
	}
}

void CharacterToBinaryTable::buildCharacterCodeToBinaryStringTable() {
	/*
	 * builds HashTable<character code, binary string>
	 */
	if (!characterCodeToBinaryStringTablePtr) {
		// we only expect around 255 unique characters
		characterCodeToBinaryStringTablePtr = make_shared<
				HashTable<unsigned int, string>>(255);
	}
	size_t i, n;
	n = binaryStringToCharacterCodeTablePtr->size();
	string binaryString;
	unsigned int characterCode;
	for (i = 0; i < n; i++) {
		if (binaryStringToCharacterCodeTablePtr->atIndex(i, binaryString,
				characterCode)) {
			characterCodeToBinaryStringTablePtr->insert(characterCode,
					binaryString);
		}
	}
}

bool CharacterToBinaryTable::characterCodeToBinaryString(
		unsigned int characterCode, string& binaryString) {
	return characterCodeToBinaryStringTablePtr->at(characterCode, binaryString);
}
bool CharacterToBinaryTable::binaryStringToCharacterCode(string binaryString,
		unsigned int characterCode) {
	return binaryStringToCharacterCodeTablePtr->at(binaryString, characterCode);
}

/*
 * Compressor Implementation
 */
void Compressor::set(shared_ptr<CharacterToBinaryTable> tablePtr) {
	characterToBinaryTablePtr = tablePtr;
}

bool Compressor::encode(istream& streamIn, ostream& streamOut) {
	/* Parsing Steps:
	 * 1. read in buffer
	 * 2. go through each character in buffer
	 *    add the character code as hash table key and value as the frequency
	 * */
	unsigned int fileSize, filePos, bufferSize, mode;
	string streamBufferIn, streamBufferOut;
	stack<string> documentStack;
	bufferSize = PRIORITY_QUEUE_PARSER_BUFFER_SIZE;
	fileSize = filePos = mode = 0;
	streamBufferIn = streamBufferOut = "";
	char bufferInChar[PRIORITY_QUEUE_PARSER_BUFFER_SIZE];
	streamIn.seekg(0, ios::end); // set the pointer to the end
	fileSize = static_cast<unsigned int>(streamIn.tellg()); // get the length of the file
	streamIn.seekg(0, ios::beg); // set the pointer to the beginning
	while (filePos < fileSize) {
		streamIn.seekg(filePos, ios::beg); // seek new position
		if (filePos + bufferSize > fileSize) {
			bufferSize = fileSize - filePos;
		}
		memset(bufferInChar, 0, sizeof(bufferInChar)); // zero out buffer
		streamIn.read(bufferInChar, bufferSize);
		streamBufferIn.append(bufferInChar, bufferSize);
		encodeBufferHandle(streamBufferIn, streamBufferOut, streamOut);
		// advance buffer
		filePos += bufferSize;
	}
	// handle the remaining buffer
	encodeBufferHandle(streamBufferIn, streamBufferOut, streamOut);
	return true;
}
bool Compressor::encodeBufferHandle(string& streamBufferIn,
		string& streamBufferOut, ostream& streamOut) {
	/* characters are placed into a hash table
	 * hashTable[CharacterCode] = CharacterFrequency
	 */
	size_t i, n;
	n = streamBufferIn.size();
	unsigned int characterCode;
	string binaryString;
	shared_ptr<unsigned int> characterFrequencyPtr;
	for (i = 0; i < n; i++) {
		characterCode =
				static_cast<unsigned int>(static_cast<int>(streamBufferIn[i]));
		if (characterToBinaryTablePtr->characterCodeToBinaryString(
				characterCode, binaryString)) {
			streamBufferOut.append(binaryString);
		}
	}
	streamBufferIn = "";
	n = streamBufferOut.size() / 8;
	for (i = 0; i < n; i++) {
		std::bitset<8> b(streamBufferOut.substr(0, 8));
		char c = b.to_ulong();
		streamOut.write(const_cast<const char*>(&c), 1);
		streamBufferOut.erase(0, 8);
	}
	// there actually is no point to the return, but maybe there will be in the future
	return true;
}

bool Compressor::decode(istream& streamIn, ostream& streamOut) {
	return true;
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
	/* input/output files are here */
	fileNameOriginal = "Speech.txt";
	fileNameEncrypt = "encrypt.data";
	fileNameDecrypt = "decrypt.txt";
	cout << "Opening the input file." << endl;
	try {
		fh.readStream(fileNameOriginal, fileStreamIn);
		fh.writeStream(fileNameEncrypt, fileStreamOut);
		cout << "Parsing input input stream as a character frequency table."
				<< endl;
		/* we pass a file stream instead of a reading the whole file
		 * into memory to reduce memory usage.
		 */
		characterPriorityQueue.fileStreamIn(fileStreamIn);
		cout << "Building the priority queue." << endl;
		characterPriorityQueue.buildPriorityQueue();
		cout << "Building the priority queue tree." << endl;
		characterPriorityQueueTree.buildTree(
				characterPriorityQueue.getPriorityQueue());
		cout << "Building the binary string table." << endl;
		characterPriorityQueueTree.buildBinaryTable();
		cout << "Encoding the input file." << endl;
		characterToBinaryTablePtr->set(
				characterPriorityQueueTree.getCharacterToBinaryTable());
		// only need for decoding
		// characterToBinaryTable.buildBinaryStringToCharacterCodeTable();
		compressorPtr->set(characterToBinaryTablePtr);
		compressorPtr->encode(fileStreamIn, fileStreamOut);
		/* encoding successful, now let's clean up */
		fh.close(fileStreamIn);
		fh.close(fileStreamOut);
		cout << "The file \"" << fileNameOriginal
				<< "\" was successfully encoded as \"" << fileNameEncrypt
				<< "\"" << endl;
	} catch (int& exceptionCode) {
		switch (exceptionCode) {
		case 1:
			cout << "[Error] Could not open the input file \""
					<< fileNameOriginal << "\"" << endl;
			break;
		case 2:
			cout << "[Error] Could not open the output file \""
					<< fileNameEncrypt << "\"" << endl;
			break;
		case 3:
			cout
					<< "[Error] Could not parse the input stream as a character frequency table."
					<< endl;
			break;
		case 4:
			cout << "[Error] Could not build the priority queue." << endl;
			break;
		case 5:
			cout << "[Error] Could not build the priority queue tree." << endl;
			break;
		case 6:
			cout << "[Error] Could not build the binary string table." << endl;
			break;
		case 7:
			cout << "[Error] Could not close the input file \""
					<< fileNameOriginal << "\"" << endl;
			break;
		case 8:
			cout << "[Error] Could not close the output file \""
					<< fileNameEncrypt << "\"" << endl;
			break;
		}
	}

	cout << "Enter any key to exit..." << endl;
	string temp;
	getline(cin, temp);
	return 0;
}
