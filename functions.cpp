#include "header.h"

void parse(string& someString, vvs &attributeTable)		// Stores data taken from an input file into a vector of vector of strings 
{
	int attributeCount = 0;
	vs vectorOfStrings;
	while (someString.length() != 0 && someString.find(',') != string::npos)
	{
		size_t pos;
		string singleAttribute;
		pos = someString.find_first_of(',');
		singleAttribute = someString.substr(0, pos);
		vectorOfStrings.push_back(singleAttribute);
		someString.erase(0, pos+1);
	}
	vectorOfStrings.push_back(someString);
	attributeTable.push_back(vectorOfStrings);
	vectorOfStrings.clear();
}

void printAttributeTable(vvs &attributeTable)	// For debugging purposes only. Prints a data table
{
	int inner, outer;
	for (outer = 0; outer < attributeTable.size(); outer++)
	{
		for (inner = 0; inner < attributeTable[outer].size(); inner++)
			cout << attributeTable[outer][inner] << "\t";
		cout << endl;
	}
}

vvs pruneTable(vvs &attributeTable, string &colName, string value)	// Prunes a table based on a column/attribute's name and the value of that attribute. Removes that column and all instances that have that value for that column
{
	int iii, jjj;
	vvs prunedTable;
	int column = -1;
	vs headerRow;
	for (iii = 0; iii < attributeTable[0].size(); iii++)
	{
		if (attributeTable[0][iii] == colName)
		{
			column = iii;
			break;
		}
	}
	for (iii = 0; iii < attributeTable[0].size(); iii++)
	{
		 if (iii != column)
		 	headerRow.push_back(attributeTable[0][iii]);
	}
	prunedTable.push_back(headerRow);
	for (iii = 0; iii < attributeTable.size(); iii++)
	{
		vs auxRow;
		if (attributeTable[iii][column] == value)
		{
			for (jjj = 0; jjj < attributeTable[iii].size(); jjj++)
			{
				if(jjj == column) {}
				else
					auxRow.push_back(attributeTable[iii][jjj]);
			}
			prunedTable.push_back(auxRow);
		}
	}
	return prunedTable;
}

node* buildDecisionTree(vvs &table, node* nodePtr, vvs &tableInfo)	// Builds the decision tree based on the table it is passed
{
	if (tableIsEmpty(table))
		return NULL;
	if (isHomogeneous(table))
	{
		nodePtr->isLeaf = true;
		nodePtr->label = table[1][table[1].size()-1];
		return nodePtr;
	}
	else
	{
		string splittingCol = decideSplittingColumn(table);
		nodePtr->splitOn = splittingCol;
		int colIndex = returnColumnIndex(splittingCol, tableInfo);
		int iii;
		for (iii = 1; iii < tableInfo[colIndex].size(); iii++)
		{
			node* newNode = (node*) new node;
			newNode->label = tableInfo[colIndex][iii];
			nodePtr->childrenValues.push_back(tableInfo[colIndex][iii]);
			newNode->isLeaf = false;
			newNode->splitOn = splittingCol;
			vvs auxTable = pruneTable(table, splittingCol, tableInfo[colIndex][iii]);
			nodePtr->children.push_back(buildDecisionTree(auxTable, newNode, tableInfo));
		}
	}
	return nodePtr;
}

bool isHomogeneous(vvs &table)		// Returns true if all instances in a subtable at a node have the same class label
{
	int iii;
	int lastCol = table[0].size() - 1;
	string firstValue = table[1][lastCol];
	for (iii = 1; iii < table.size(); iii++)
	{
		if (firstValue != table[iii][lastCol])
			return false;
	}
	return true;
}

vi countDistinct(vvs &table, int column)		// Returns a vector of integers containing the counts of all the various values of an attribute/column
{
	vs vectorOfStrings;
	vi counts;
	bool found = false;
	int foundIndex;
	for (int iii = 1; iii < table.size(); iii++)
	{
		for (int jjj = 0; jjj < vectorOfStrings.size(); jjj++)
		{
			if (vectorOfStrings[jjj] == table[iii][column])
			{
				found = true;
				foundIndex = jjj;
				break;
			}
			else
				found = false;
		}
		if (!found)
		{
			counts.push_back(1);
			vectorOfStrings.push_back(table[iii][column]);
		}
		else
			counts[foundIndex]++;
	}
	int sum = 0;
	for (int iii = 0; iii < counts.size(); iii++)
		sum += counts[iii];
	counts.push_back(sum);
	return counts;
}

string decideSplittingColumn(vvs &table)		// Returns the column on which to split on. Decision of column is based on entropy
{
	int column, iii;
	double minEntropy = DBL_MAX;
	int splittingColumn = 0;
	vi entropies;
	for (column = 0; column < table[0].size() - 1; column++)
	{
		string colName = table[0][column];
		msi tempMap;
		vi counts = countDistinct(table, column);
		vd attributeEntropy;
		double columnEntropy = 0.0;
		for (iii = 1; iii < table.size()-1; iii++)
		{
			double entropy = 0.0;
			if (tempMap.find(table[iii][column]) != tempMap.end()) 	// IF ATTRIBUTE IS ALREADY FOUND IN A COLUMN, UPDATE IT'S FREQUENCY
				tempMap[table[iii][column]]++;
			else							// IF ATTRIBUTE IS FOUND FOR THE FIRST TIME IN A COLUMN, THEN PROCESS IT AND CALCULATE IT'S ENTROPY
			{
				tempMap[table[iii][column]] = 1;
				vvs tempTable = pruneTable(table, colName, table[iii][column]);
				vi classCounts = countDistinct(tempTable, tempTable[0].size()-1);
				int jjj, kkk;
				for (jjj = 0; jjj < classCounts.size(); jjj++)
				{
					double temp = (double) classCounts[jjj];
					entropy -= (temp/classCounts[classCounts.size()-1])*(log(temp/classCounts[classCounts.size()-1]) / log(2));
				}
				attributeEntropy.push_back(entropy);
				entropy = 0.0;
			}
		}
		for (iii = 0; iii < counts.size() - 1; iii++)
		{
			columnEntropy += ((double) counts[iii] * (double) attributeEntropy[iii]);
		}
		columnEntropy = columnEntropy / ((double) counts[counts.size() - 1]);
		if (columnEntropy <= minEntropy)
		{
			minEntropy = columnEntropy;
			splittingColumn = column;
		}
	}
	return table[0][splittingColumn];
}

int returnColumnIndex(string &columnName, vvs &tableInfo)		// Returns the index of a column in a subtable
{
	int iii;
	for (iii = 0; iii < tableInfo.size(); iii++)
	{
		if (tableInfo[iii][0] == columnName)
		{
			return iii;
		}
	}
	return -1;
}

bool tableIsEmpty(vvs &table)			// Returns true if a subtable is empty
{
	return (table.size() == 1);
}

void printDecisionTree(node* nodePtr)		// For degubbing purposes only. Recursively prints decision tree
{
	if(nodePtr == NULL)
		return;
	if (!nodePtr->children.empty())
        {
		cout << " Value: " << nodePtr->label << endl;
		cout << "Split on: " << nodePtr->splitOn;
		int iii;
		for (iii = 0; iii < nodePtr->children.size(); iii++)
		{   
			cout << "\t";
			printDecisionTree(nodePtr->children[iii]);
		}
		return;
        }
	else
	{
		cout << "Predicted class = " << nodePtr->label;
		return;
	}
}

string testDataOnDecisionTree(vs &singleLine, node* nodePtr, vvs &tableInfo, string defaultClass)	// Runs a single instance of the test data through the decision tree. Returns the predicted class label
{
	string prediction;
	while (nodePtr->isLeaf != true && !nodePtr->children.empty())
	{
		int index = returnColumnIndex(nodePtr->splitOn, tableInfo);
		string value = singleLine[index];
		int childIndex = returnIndexOfVector(nodePtr->childrenValues, value);
		nodePtr = nodePtr->children[childIndex];
		if (nodePtr == NULL)
		{
			prediction = defaultClass;
			break;
		}
		prediction = nodePtr->label;
	}
	return prediction;
}

int returnIndexOfVector(vs &stringVector, string value)		// Returns the index of a string in a vector of strings
{
	int iii;
	for (iii = 0; iii < stringVector.size(); iii++)
	{
		if (stringVector[iii] == value)
		{
			return iii;
		}
	}
	return -1;
}

double printPredictionsAndCalculateAccuracy(vs &givenData, vs &predictions)		// Outputs the predictions to file and returns the accuracy of the classification
{
	ofstream outputFile;
	outputFile.open("decisionTreeOutput.txt");
	int correct = 0;
	outputFile << setw(3) << "#" << setw(16) << "Given Class" << setw(31) << right << "Predicted Class" << endl;
	outputFile << "--------------------------------------------------" << endl;
	for (int iii = 0; iii < givenData.size(); iii++)
	{
		outputFile << setw(3) << iii+1 << setw(16) << givenData[iii];
		if (givenData[iii] == predictions[iii])
		{
			correct++;
			outputFile << "  ------------  ";
		}
		else
			outputFile << "  xxxxxxxxxxxx  ";
		outputFile << predictions[iii] << endl;
	}
	outputFile << "--------------------------------------------" << endl;
	outputFile << "Total number of instances in test data = " << givenData.size() << endl;
	outputFile << "Number of correctly predicted instances = " << correct << endl;
	outputFile.close();
	return (double) correct/50 * 100;
}

vvs generateTableInfo(vvs &dataTable)		// Generates information about the table in a vector of vector of stings
{
	vvs tableInfo;
	for (int iii = 0; iii < dataTable[0].size(); iii++)
	{
		vs tempInfo;
		msi tempMap;
		for (int jjj = 0; jjj < dataTable.size(); jjj++)
		{
			if (tempMap.count(dataTable[jjj][iii]) == 0)
			{
				tempMap[dataTable[jjj][iii]] = 1;
				tempInfo.push_back(dataTable[jjj][iii]);
			}
			else
				tempMap[dataTable[jjj][iii]]++;
		}
		tableInfo.push_back(tempInfo);
	}
	return tableInfo;
}

string returnMostFrequentClass(vvs &dataTable)		// Returns the most frequent class from the training data. This class is used as the default class during the testing phase
{
	msi trainingClasses;            // Stores the classlabels and their frequency
	for (int iii = 1; iii < dataTable.size(); iii++)
	{
		if (trainingClasses.count(dataTable[iii][dataTable[0].size()-1]) == 0)
			trainingClasses[dataTable[iii][dataTable[0].size()-1]] = 1;
		else
			trainingClasses[dataTable[iii][dataTable[0].size()-1]]++;
	}   
	msi::iterator mapIter;
	int highestClassCount = 0;
	string mostFrequentClass;
	for (mapIter = trainingClasses.begin(); mapIter != trainingClasses.end(); mapIter++)
	{
		if (mapIter->second >= highestClassCount)
		{
			highestClassCount = mapIter->second;
			mostFrequentClass = mapIter->first;
		}   
	}
	return mostFrequentClass;
}
