#include"IPCMessage.h"
pid_t IPCMessage::GetSource()
{
	return msg.source;
}
pid_t IPCMessage::GetDestination()
{
	return msg.destination;
}
IPCMessage::IPCMessage(Message _Msg)
{
	msg = _Msg;
}
uint64_t IPCMessage::GetTimeStamp()
{
	return msg.timeStamp;
}
bool IPCMessage::operator<(const IPCMessage& _Msg)
{
	return msg.timeStamp <	_Msg.msg.timeStamp; 
}
bool IPCMessage::operator<=(const IPCMessage& _Msg)
{
	return msg.timeStamp <=	_Msg.msg.timeStamp; 
}
bool IPCMessage::operator>(const IPCMessage& _Msg)
{
	return !(*this<_Msg);
}
bool IPCMessage::operator>=(const IPCMessage& _Msg)
{
	return !(*this<=_Msg);
}
