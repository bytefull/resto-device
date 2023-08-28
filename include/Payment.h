#ifndef PAYMENT_H
#define PAYMENT_H

#include <Arduino.h>
#include <functional>
#include "Order.h"

class Payment {

public:
  enum class PaymentError {
    Success,
    ServerError,
    InvalidOrder,
    ConnectionError,
    RequestError,
    StatusCodeError,
    InvalidResponseHeaders,
    DeserializeResponseError,
  };

  void initiate(const Order& order);
  void onResult(std::function<void(PaymentError)> callback);

private:
  std::function<void(PaymentError)> resultCallback;

};

#endif // PAYMENT_H
