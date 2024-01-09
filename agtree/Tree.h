#pragma once

#include "db.h"

class Node {
public:
    Node() = default;
};

class Tree {
private:
    DB* db;

public:
    Node root;
    Tree(DB* db_);
};
