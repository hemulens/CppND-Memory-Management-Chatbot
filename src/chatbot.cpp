#include "chatbot.h"

#include <algorithm>
#include <ctime>
#include <iostream>
#include <random>
#include <string>
#include <memory>
#include <iostream>

#include "chatlogic.h"
#include "graphedge.h"
#include "graphnode.h"

// constructor WITHOUT memory allocation
ChatBot::ChatBot() {
  // invalidate data handles
  _image = nullptr;
  _chatLogic = nullptr;
  _rootNode = nullptr;
}

// constructor WITH memory allocation
ChatBot::ChatBot(std::string filename) {
  std::cout << "ChatBot Constructor" << std::endl;

  // invalidate data handles
  _chatLogic = nullptr;
  _rootNode = nullptr;

  // load image into heap memory
  _image = new wxBitmap(filename, wxBITMAP_TYPE_PNG);
}

ChatBot::~ChatBot() {
  std::cout << "ChatBot Destructor" << std::endl;

  // deallocate heap memory
  if (_image != NULL) {  // Attention: wxWidgets used NULL and not nullptr
    delete _image;
    std::cout << "_image memory at heap address " << _image << " and stack address " << &_image << " deleted" << std::endl;
    _image = NULL;
  } else {  // (Sergei) added for testing
    std::cout << "ChatBot Destructor: _image is already equal to NULL – (Sergei/experimental)" << std::endl;
  }
}

//// STUDENT CODE
////
// 2) Copy constructor
ChatBot::ChatBot(const ChatBot &source) {
  _image = new wxBitmap(*source._image);  // deep copy; allocating memory on the heap using wxBitmap copy constructor (https://docs.wxwidgets.org/trunk/classwx_bitmap.html).
  _chatLogic = source._chatLogic;  // shallow copy, as the resource isn't owned
  _chatLogic->SetChatbotHandle(this);  // setting chatbot handle to the current object (still confused and not sure if I need to do it... To tutor: please help me understand if this is needed). Why I included it: saw two _chaBot objects on the schema and thought this class might pass ownership to ChatLogic; also, found the method SetChatbotHandle in the header file which confirmed my hypothesis
  _currentNode = source._currentNode;  // shallow copy, as the resource isn't owned
  _rootNode = source._rootNode;  // shallow copy, as the resource isn't owned
  std::cout << "ChatBot Copy Constructor -> copied content of instance " << &source << " to instance " << this << std::endl;
}
// 3) Assignment operator constructor
ChatBot &ChatBot::operator=(const ChatBot &source) {
  if (this == &source) {
    return *this;
  }
  delete _image;  // assuming that an object lived before
  _image = new wxBitmap(*source._image);  // deep copy; allocating memory on the heap using wxBitmap copy constructor (https://docs.wxwidgets.org/trunk/classwx_bitmap.html)
  _chatLogic = source._chatLogic;  // shallow copy, as the resource isn't owned
  _currentNode = source._currentNode;  // shallow copy, as the resource isn't owned
  _rootNode = source._rootNode;  // shallow copy, as the resource isn't owned
  std::cout << "ChatBot Copy Assignment Constructor -> Assigned content of instance " << &source << " to instance " << this << std::endl;
  return *this;
}
// 4) Move constructor
ChatBot::ChatBot(ChatBot &&source) {
  _image = source._image;
  _chatLogic = source._chatLogic;
  _currentNode = source._currentNode;
  _rootNode = source._rootNode;
  _chatLogic->SetChatbotHandle(this);
  source._image = NULL;  // Sergei: NULL (not nullptr) to be consistent with the destructor
  source._chatLogic = nullptr;
  source._currentNode = nullptr;
  source._rootNode = nullptr;
  std::cout << "ChatBot Move Constructor -> Moved instance (constructor) " << &source << " to " << this << std::endl;
}
// 5) Move assignment operator
ChatBot &ChatBot::operator=(ChatBot &&source) {
  if (this == &source) {
    return *this;
  }
  delete _image;
  _image = source._image;
  _chatLogic = source._chatLogic;
  _currentNode = source._currentNode;
  _rootNode = source._rootNode;
  _chatLogic->SetChatbotHandle(this);
  source._image = NULL;  // Sergei: NULL (not nullptr) to be consistent with the destructor
  source._chatLogic = nullptr;
  source._currentNode = nullptr;
  source._rootNode = nullptr;
  std::cout << "ChatBot Move Assignment Constructor -> Moved instance (assignment) " << &source << " to " << this << std::endl;
  return *this;
}
////
//// EOF STUDENT CODE

void ChatBot::ReceiveMessageFromUser(std::string message) {
  // loop over all edges and keywords and compute Levenshtein distance to query
  typedef std::pair<GraphEdge *, int> EdgeDist;
  std::vector<EdgeDist> levDists;  // format is <ptr,levDist>

  for (size_t i = 0; i < _currentNode->GetNumberOfChildEdges(); ++i) {
    GraphEdge *edge = _currentNode->GetChildEdgeAtIndex(i);
    for (auto keyword : edge->GetKeywords()) {
      EdgeDist ed{edge, ComputeLevenshteinDistance(keyword, message)};
      levDists.push_back(ed);
    }
  }

  // select best fitting edge to proceed along
  GraphNode *newNode;
  if (levDists.size() > 0) {
    // sort in ascending order of Levenshtein distance (best fit is at the top)
    std::sort(levDists.begin(), levDists.end(), [](const EdgeDist &a, const EdgeDist &b) { return a.second < b.second; });
    newNode = levDists.at(0).first->GetChildNode();  // after sorting the best edge is at first position
  } else {
    // go back to root node
    newNode = _rootNode;
  }

  // tell current node to move chatbot to new node
  _currentNode->MoveChatbotToNewNode(newNode);
}

void ChatBot::SetCurrentNode(GraphNode *node) {
  // update pointer to current node
  _currentNode = node;

  // select a random node answer (if several answers should exist)
  std::vector<std::string> answers = _currentNode->GetAnswers();
  std::mt19937 generator(int(std::time(0)));
  std::uniform_int_distribution<int> dis(0, answers.size() - 1);
  std::string answer = answers.at(dis(generator));

  // send selected node answer to user
  _chatLogic->SendMessageToUser(answer);
}

int ChatBot::ComputeLevenshteinDistance(std::string s1, std::string s2) {
  // convert both strings to upper-case before comparing
  std::transform(s1.begin(), s1.end(), s1.begin(), ::toupper);
  std::transform(s2.begin(), s2.end(), s2.begin(), ::toupper);

  // compute Levenshtein distance measure between both strings
  const size_t m(s1.size());
  const size_t n(s2.size());

  if (m == 0)
    return n;
  if (n == 0)
    return m;

  size_t *costs = new size_t[n + 1];

  for (size_t k = 0; k <= n; k++)
    costs[k] = k;

  size_t i = 0;
  for (std::string::const_iterator it1 = s1.begin(); it1 != s1.end(); ++it1, ++i) {
    costs[0] = i + 1;
    size_t corner = i;

    size_t j = 0;
    for (std::string::const_iterator it2 = s2.begin(); it2 != s2.end(); ++it2, ++j) {
      size_t upper = costs[j + 1];
      if (*it1 == *it2) {
        costs[j + 1] = corner;
      } else {
        size_t t(upper < corner ? upper : corner);
        costs[j + 1] = (costs[j] < t ? costs[j] : t) + 1;
      }

      corner = upper;
    }
  }

  int result = costs[n];
  delete[] costs;

  return result;
}