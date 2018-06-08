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
	bool readStream(string fileName, ifstream& fileStream) throw (unsigned int);
	bool writeStream(string fileName, ofstream& fileStream) throw (unsigned int);
	bool writeString(string fileName, string stringValue);
	bool close(ifstream& fileStream) throw (unsigned int);
	bool close(ofstream& fileStream) throw (unsigned int);
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
	T at(K key);
	T atIndex(size_t index);
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
};

/**
 * @class CharacterPriorityQueueCompare
 * Function object for performing comparisons. Uses operator< on type T. \n
 */
template<class T>
class CharacterPriorityQueueCompare {
	bool operator()(const T &lhs, const T &rhs) const {
		return *lhs < *rhs;
	}
};

/**
 * The priority queue type is long so we make it into an alias
 * using is used rather than typedef because of this forum:
 * https://stackoverflow.com/questions/10747810/what-is-the-difference-between-typedef-and-using-in-c11
 */
using priorityQueueType = std::priority_queue<shared_ptr<CharacterFrequencyNode>, vector<shared_ptr<CharacterFrequencyNode>>, CharacterPriorityQueueCompare<shared_ptr<CharacterFrequencyNode>>>;

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
public:
	CharacterPriorityQueueTreeNode() {

	}
	virtual ~CharacterPriorityQueueTreeNode() {

	}
	virtual bool isLeaf();
	virtual bool isBranch();
};

class CharacterPriorityQueueTreeLeaf: CharacterPriorityQueueTreeNode {
private:
	shared_ptr<CharacterFrequencyNode> characterFrequencyNode;
public:
	CharacterPriorityQueueTreeLeaf() {

	}
	~CharacterPriorityQueueTreeLeaf() {

	}
	bool isLeaf();
	bool isBranch();
	shared_ptr<CharacterFrequencyNode> getCharacterNode();
};

class CharacterPriorityQueueTreeBranch: CharacterPriorityQueueTreeNode {
private:
	unique_ptr<CharacterPriorityQueueTreeNode> left, right;
public:
	CharacterPriorityQueueTreeBranch() {

	}
	~CharacterPriorityQueueTreeBranch() {

	}
	bool isLeaf();
	bool isBranch();
	unique_ptr<CharacterPriorityQueueTreeNode> getLeft();
	unique_ptr<CharacterPriorityQueueTreeNode> getRight();
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
bool FileHandler::readStream(string fileName, ifstream& fileStream)
		throw (unsigned int) {
	fileStream.open(fileName, ios::binary);
	if (fileStream.is_open()) {
		return true;
	}
	throw 0;
	return false;
}

bool FileHandler::writeStream(string fileName, ofstream& fileStream)
		throw (unsigned int) {
	fileStream.open(fileName);
	if (fileStream.is_open()) {
		return true;
	}
	throw 1;
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

bool FileHandler::close(ifstream& fileStream) throw (unsigned int) {
	try {
		fileStream.close();
	} catch (...) {
		throw 0;
		return false;
	}
	return true;
}

bool FileHandler::close(ofstream& fileStream) throw (unsigned int) {
	try {
		fileStream.close();
	} catch (...) {
		throw 0;
		return false;
	}
	return true;
}

/*
 * HashTable Implementation
 */
template<class K, class T>
bool HashTable<K, T>::insert(K key, T val) {
	unsigned int attempts = insertAttempts;
	K keyOriginal = key;
	size_t keyInt = hashT(key);
	bool flag = false;
	for (; attempts > 0; attempts--) {
		if (table[keyInt] == nullptr) {
			table[keyInt] = make_unique<pair<K, T>>(keyOriginal, val);
			flag = true;
			break;
		}
		keyInt = hashSize(keyInt);
	}
	return flag;
}
template<class K, class T>
T HashTable<K, T>::at(K key) {
	unsigned int attempts = insertAttempts;
	K keyOriginal = key;
	size_t keyInt = hashT(key);
	T ret { };
	for (; attempts > 0; attempts--) {
		if (table[keyInt] != nullptr && table[keyInt]->first == keyOriginal) {
			ret = table[keyInt]->second;
			break;
		}
		keyInt = hashSize(keyInt);
	}
	if (attempts == 0) {
		throw out_of_range("");
	}
	return ret;
}
template<class K, class T>
T HashTable<K, T>::atIndex(size_t index) {
	pair<K, T>* temp = table[index];
	if (temp != nullptr) {
		return temp->second;
	} else {
		throw invalid_argument("");
	}
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
	return ref(priorityQueue);
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
	} catch (unsigned int &exceptionCode) {
		switch (exceptionCode) {
		case 1:
			cout << "[Error] Could not open either the input file \""
					<< fileNameOriginal << "\"" << endl;
			break;
		case 2:
			cout << "[Error] Could not open either the output file \""
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
		}
	}

	cout << "Enter any key to exit..." << endl;
	string temp;
	getline(cin, temp);
	return 0;
}
