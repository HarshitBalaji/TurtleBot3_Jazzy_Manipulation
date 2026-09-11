#include <chrono>

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist_stamped.hpp"

using namespace std::chrono_literals;


class TurtleBotController : public rclcpp::Node
{
public:

    TurtleBotController()
        : Node("turtlebot_controller")
    {
        cmd_vel_publisher_ =
            this->create_publisher<geometry_msgs::msg::TwistStamped>(
                "/diff_drive_controller/cmd_vel",
                10
            );

        timer_ = this->create_wall_timer(
            100ms,
            std::bind(
                &TurtleBotController::control_loop,
                this
            )
        );

        RCLCPP_INFO(
            this->get_logger(),
            "TurtleBot controller started"
        );
    }


private:

    void control_loop()
    {
        geometry_msgs::msg::TwistStamped msg;

        // Timestamp
        msg.header.stamp = this->now();

        // Move forward at 0.2 m/s
        msg.twist.linear.x = 0.2;

        // No rotation
        msg.twist.angular.z = 0.0;

        cmd_vel_publisher_->publish(msg);
    }


    rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr
        cmd_vel_publisher_;

    rclcpp::TimerBase::SharedPtr timer_;
};


int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);

    auto node =
        std::make_shared<TurtleBotController>();

    rclcpp::spin(node);

    rclcpp::shutdown();

    return 0;
}