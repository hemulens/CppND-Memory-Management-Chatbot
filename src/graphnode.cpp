#include "graphnode.h"

#include "graphedge.h"

GraphNode::GraphNode(int id) {
  _id = id;
}

GraphNode::~GraphNode() {
  //// STUDENT CODE
  ////
  // Sergei: TASK 0 – destructor must not delete _chatBot, because GraphNode does not own _chatBot.
  // delete _chatBot;  // Sergei: disabled deletion
  ////
  //// EOF STUDENT CODE
}

void GraphNode::AddToken(std::string token) {
  _answers.push_back(token);
}

void GraphNode::AddEdgeToParentNode(GraphEdge *edge) {
  _parentEdges.push_back(edge);
}

void GraphNode::AddEdgeToChildNode(std::unique_ptr<GraphEdge> edge) {  // Sergei: prior to this scenario, I considered passing an r-value of the unique pointer, i.e. &&edge.
  _childEdges.push_back(std::move(edge));
}

//// STUDENT CODE
////
void GraphNode::MoveChatbotHere(ChatBot chatbot) {
  _chatBot = std::move(chatbot);
  _chatBot.SetCurrentNode(this);
}

void GraphNode::MoveChatbotToNewNode(GraphNode *newNode) {
  newNode->MoveChatbotHere(std::move(_chatBot));
  // _chatBot = nullptr;  // invalidate pointer at source  // Sergei: no need to invalidate the pointer, as we use a variable
}
////
//// EOF STUDENT CODE

GraphEdge *GraphNode::GetChildEdgeAtIndex(int index) {
  //// STUDENT CODE
  ////

  return _childEdges[index].get();

  ////
  //// EOF STUDENT CODE
}