#include <Arduino.h>
#include "Order.h"

Order::Order(const char *customerId, int restaurantId, int amount, int timestamp) {
  this->customerId = customerId;
  this->restaurantId = restaurantId;
  this->amount = amount;
  this->timestamp = timestamp;
}

const char* Order::getCustomerId() const {
  return customerId;
}

int Order::getRestaurantId() const {
  return restaurantId;
}

int Order::getAmount() const {
  return amount;
}

int Order::getTimestamp() const {
  return timestamp;
}

void Order::setCustomerId(const char *customerId) {
  this->customerId = customerId;
}

void Order::setRestaurantId(int restaurantId) {
  this->restaurantId = restaurantId;
}

void Order::setAmount(int amount) {
  this->amount = amount;
}

void Order::setTimestamp(int timestamp) {
  this->timestamp = timestamp;
}
