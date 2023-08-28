#ifndef ORDER_H
#define ORDER_H

#include <Arduino.h>

class Order {

public:
  Order(const char *customerId, int restaurantId, int amount, int timestamp);

  const char* getCustomerId() const;
  int getRestaurantId() const;
  int getAmount() const;
  int getTimestamp() const;

  void setCustomerId(const char *customerId);
  void setRestaurantId(int restaurantId);
  void setAmount(int amount);
  void setTimestamp(int timestamp);

private:
  const char *customerId;
  int restaurantId;
  int amount;
  int timestamp;

};

#endif // ORDER_H
