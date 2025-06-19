#pragma once
#include <functional>

#include "Command.h"

class LambdaCommand : public Command
{
public:
    explicit LambdaCommand(std::function<void()> func) : m_Func(std::move(func)) {}
    void Execute() override { m_Func(); }

private:
    std::function<void()> m_Func;
};
