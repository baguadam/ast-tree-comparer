#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "node_reader.h"

std::unique_ptr<Node> NodeReader::readASTDump(const std::string& fileName) {
    std::vector<Node*> nodeStack;
    std::ifstream file(fileName);
    std::string line;

    while (std::getline(file, line)) {
        int depth = 0;
        while (line[depth] == ' ') {
            ++depth;
        }

        std::string nodeName;
        std::string nodeValue;
        std::istringstream iss(line);

        // reading the first word into nodeName
        iss >> nodeName;

        // rest of the line into nodeValue
        std::getline(iss, nodeValue);
        // removing whitespaces
        nodeValue.erase(nodeValue.begin(), std::find_if(nodeValue.begin(), nodeValue.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        nodeValue.erase(std::find_if(nodeValue.rbegin(), nodeValue.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), nodeValue.end()); 

        auto node = std::make_unique<Node>();
        node->name = nodeName;
        node->value = nodeValue;

        while (nodeStack.size() > depth) {
            nodeStack.pop_back();
        }

        node->parent = nodeStack.empty() ? nullptr : nodeStack.back();
        if (node->parent) {
            node->parent->children.push_back(std::move(node));
        }

        nodeStack.push_back(node.get());
    }
    
    return std::unique_ptr<Node>(nodeStack.front());
}