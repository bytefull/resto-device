#include <Arduino.h>

#include <STM32Ethernet.h>
#include <SSLClient.h>
#include <ArduinoJson.h>

#include "Payment.h"
#include "certificates.h"

void Payment::initiate(const Order& order) {
  PaymentError paymentResult;

  EthernetClient tcpClient;
  SSLClient sslClient(tcpClient, TAs, (size_t)TAs_NUM, A5, SSLClient::SSL_INFO);

  DeserializationError deserializeError;

  StaticJsonDocument<512> requestJsonDoc;
  StaticJsonDocument<128> responseJsonDoc;
  char requestJsonString[512] = {0};
  char httpStatusCode[32] = {0};

  const char *server = "restoken.azurewebsites.net";
  const uint16_t port = 443;

  // 0. Sanity check the order
  if (&order == nullptr) {
    paymentResult = PaymentError::InvalidOrder;
    goto callback_and_exit;
  }

  // 1. Connect to HTTPS server
  if (sslClient.connect(server, port) == 0) {
    paymentResult = PaymentError::ConnectionError;
    goto callback_and_exit;
  }

  // 2. Serialize Order object to a JSON string
  requestJsonDoc["customer_id"] = order.getCustomerId();
  requestJsonDoc["restaurant_id"] = order.getRestaurantId();
  requestJsonDoc["amount"] = order.getAmount();
  requestJsonDoc["timestamp"] = order.getTimestamp();
  serializeJson(requestJsonDoc, requestJsonString);

  // 3. Send an HTTPS POST request to endpoint
  sslClient.println("POST /api/v1/payment HTTP/1.1");
  sslClient.println("User-Agent: SSLClientOverEthernet");
  sslClient.print("Host: ");
  sslClient.println(server);
  sslClient.println(F("Connection: close"));
  sslClient.print("Content-Length: ");
  sslClient.println(strlen(requestJsonString));
  sslClient.println();
  sslClient.println(requestJsonString);
  if (sslClient.println() == 0) {
    sslClient.stop();
    paymentResult = PaymentError::RequestError;
    goto callback_and_exit;
  }

  // 4. Check for HTTP response status code
  sslClient.readBytesUntil('\r', httpStatusCode, sizeof(httpStatusCode));
  if (strcmp(httpStatusCode + strlen("HTTP/1.0 "), "200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(httpStatusCode);
    paymentResult = PaymentError::StatusCodeError;
    goto callback_and_exit;
  }

  // 5. Skip HTTP response headers
  if (!sslClient.find("\r\n\r\n")) {
    sslClient.stop();
    paymentResult = PaymentError::InvalidResponseHeaders;
    goto callback_and_exit;
  }

  // 6. De-serialize response body to create JSON object
  deserializeError = deserializeJson(responseJsonDoc, sslClient);
  if (deserializeError) {
    sslClient.stop();
    paymentResult = PaymentError::DeserializeResponseError;
    goto callback_and_exit;
  }

  // 7. Disconnect from HTTPS server
  sslClient.stop();

  // 8. Extract values from JSON response
  if (strcmp(responseJsonDoc["status"].as<const char*>(), "success") != 0) {
    paymentResult = PaymentError::ServerError;
    goto callback_and_exit;
  }

  // 8. If we made it here then we're good
  paymentResult = PaymentError::Success;

  // Raise result callback with payment result as a parameter
  callback_and_exit:
  if (resultCallback) {
    resultCallback(paymentResult);
  }
}

void Payment::onResult(std::function<void(Payment::PaymentError)> callback) {
  resultCallback = callback;
}
