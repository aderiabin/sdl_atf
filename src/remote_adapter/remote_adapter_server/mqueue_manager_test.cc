#include <gtest/gtest.h>
#include "mqueue_manager.h"
#include <string>

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

namespace mq_wrappers_tests {

using namespace mq_wrappers;
const int kSuccess = MQueueManager::SUCCESS;
const int prio = 0;
const std::string kMqPath = "/test_mq";
const std::string kData = "testing";
const char k_max_queue_message_size = 10;

class MQueueManagerTest : public ::testing::Test {
 public:
  void SetUp() {
    EXPECT_EQ(mq_manager.MqOpen(kMqPath), kSuccess);
    EXPECT_EQ(errno, 0);
  }

 protected:
  MQueueManager mq_manager;
};

TEST_F(MQueueManagerTest, MqOpen_CreateMqueue_SUCCESS) {
  // Check that kMqPath actually exists
  mq_open(kMqPath.c_str(), O_CREAT | O_EXCL);
  EXPECT_EQ(errno, EEXIST);
}

TEST_F(MQueueManagerTest, MqSend_SendMessageViaMq_SUCCESS) {
  // Check that the mq message send successfully
  mq_manager.MqSend(kMqPath, kData);

  char buffer[k_max_queue_message_size];
  EXPECT_EQ(mq_receive(mq_manager.handles_[kMqPath], buffer, sizeof(buffer), 0),
            kData.length());
  // Check that the message which sended via MQueueManager has no difference
  // after receive message from mqueue.If 'strncmp' function return '0'
  // that will success
  EXPECT_EQ(strncmp(kData.c_str(), buffer, kData.length()), 0);
}

TEST_F(MQueueManagerTest, MqReceive_ReceiveMessageFromMqueue_SUCCESS) {
  mq_send(mq_manager.handles_[kMqPath],
          kData.c_str(),
          sizeof(kData.c_str()) - 1,
          prio);
  auto receive_message = mq_manager.MqReceive(kMqPath);

  // Check that the sended message same as receive
  EXPECT_EQ(std::get<0>(receive_message), kData);
}

TEST_F(MQueueManagerTest, MqClose_CloseMqueue_SUCCESS) {
  // Check that the Mqueue was created
  mq_open(kMqPath.c_str(), O_CREAT | O_EXCL);
  EXPECT_EQ(errno, EEXIST);

  auto desc = mq_manager.handles_[kMqPath];
  mq_manager.MqClose(kMqPath);
  mq_close(desc);
  // Check that the Mqueue is close
  EXPECT_EQ(errno, EBADF);
}

}  // namespace mq_wrappers_test
