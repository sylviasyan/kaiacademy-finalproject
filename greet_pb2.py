# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: greet.proto
"""Generated protocol buffer code."""
from google.protobuf import descriptor as _descriptor
from google.protobuf import descriptor_pool as _descriptor_pool
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()

# has diff request and responses


# DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n\x0bgreet.proto')

# _builder.BuildMessageAndEnumDescriptors(DESCRIPTOR, globals())
# _builder.BuildTopDescriptorsAndMessages(DESCRIPTOR, 'greet_pb2', globals())
# if _descriptor._USE_C_DESCRIPTORS == False:

#   DESCRIPTOR._options = None
# @@protoc_insertion_point(module_scope)


DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n\x0bgreet.proto\x12\x05greet\".\n\x0cHelloRequest\x12\x0c\n\x04name\x18\x01 \x01(\t\x12\x10\n\x08greeting\x18\x02 \x01(\t\"\x1d\n\nHelloReply\x12\x0f\n\x07message\x18\x01 \x01(\t\"E\n\x0c\x44\x65layedReply\x12\x0f\n\x07message\x18\x01 \x01(\t\x12$\n\x07request\x18\x02 \x03(\x0b\x32\x13.greet.HelloRequest2\xff\x01\n\x07Greeter\x12\x32\n\x08SayHello\x12\x13.greet.HelloRequest\x1a\x11.greet.HelloReply\x12;\n\x0fParrotSaysHello\x12\x13.greet.HelloRequest\x1a\x11.greet.HelloReply0\x01\x12\x43\n\x15\x43hattyClientSaysHello\x12\x13.greet.HelloRequest\x1a\x13.greet.DelayedReply(\x01\x12>\n\x10InteractingHello\x12\x13.greet.HelloRequest\x1a\x11.greet.HelloReply(\x01\x30\x01\x62\x06proto3')



_HELLOREQUEST = DESCRIPTOR.message_types_by_name['HelloRequest']
_HELLOREPLY = DESCRIPTOR.message_types_by_name['HelloReply']
_DELAYEDREPLY = DESCRIPTOR.message_types_by_name['DelayedReply']
HelloRequest = _reflection.GeneratedProtocolMessageType('HelloRequest', (_message.Message,), {
  'DESCRIPTOR' : _HELLOREQUEST,
  '__module__' : 'greet_pb2'
  # @@protoc_insertion_point(class_scope:greet.HelloRequest)
  })
_sym_db.RegisterMessage(HelloRequest)

HelloReply = _reflection.GeneratedProtocolMessageType('HelloReply', (_message.Message,), {
  'DESCRIPTOR' : _HELLOREPLY,
  '__module__' : 'greet_pb2'
  # @@protoc_insertion_point(class_scope:greet.HelloReply)
  })
_sym_db.RegisterMessage(HelloReply)

DelayedReply = _reflection.GeneratedProtocolMessageType('DelayedReply', (_message.Message,), {
  'DESCRIPTOR' : _DELAYEDREPLY,
  '__module__' : 'greet_pb2'
  # @@protoc_insertion_point(class_scope:greet.DelayedReply)
  })
_sym_db.RegisterMessage(DelayedReply)

_GREETER = DESCRIPTOR.services_by_name['Greeter']
if _descriptor._USE_C_DESCRIPTORS == False:

  DESCRIPTOR._options = None
  _HELLOREQUEST._serialized_start=22
  _HELLOREQUEST._serialized_end=68
  _HELLOREPLY._serialized_start=70
  _HELLOREPLY._serialized_end=99
  _DELAYEDREPLY._serialized_start=101
  _DELAYEDREPLY._serialized_end=170
  _GREETER._serialized_start=173
  _GREETER._serialized_end=428
# @@protoc_insertion_point(module_scope)
