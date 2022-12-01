from concurrent import futures #allows us to set number of workers on our server
import time #useful later on for streaming

import grpc
import greet_pb2
import greet_pb2_grpc


# from walkthru, register_implant() has a lot of params, Cam thinks these params are the values that
# should be in the messages in the proto file. If they are then we can simply change the params in the existing functions

class GreeterServicer(greet_pb2_grpc.GreeterServicer):
    # going to implement all of the different calls that I've defined in greet.proto file
    
    #unary 
    def SayHello(self, request, context):
        # very similar to a REST API call: you make a request and receive a response
        print("SayHello Request Made:")
        print(request)
        hello_reply = greet_pb2.HelloReply()
        hello_reply.message = f"{request.greeting} {request.name}"
        # example of how she ran this and tested is @15:55 in the video
        return hello_reply
    
    #server-side streaming
    def ParrotSaysHello(self, request, context):
        print("ParrotSaysHello Request Made:")
        print(request)

        for i in range(3): # why 3??
            # 3 replies for 1 request. Why did she choose that?
            hello_reply = greet_pb2.HelloReply()
            hello_reply.message = f"{request.greeting} {request.name} {i + 1}"
             # this i + 1 here is just so we can see if the messages are coming back in the right order
            yield hello_reply # here we use yield instead of return
            time.sleep(3) # we only add this so we don't get all our messages in one go

    #client-side streaming
    def ChattyClientSaysHello(self, request_iterator, context):
        delayed_reply = greet_pb2.DelayedReply()
        for request in request_iterator:
            print("ChattyClientSaysHello Request Made:")
            print(request)
            delayed_reply.request.append(request)

        delayed_reply.message = f"You have sent {len(delayed_reply.request)} messages. Please expect a delayed response."
        return delayed_reply

    #bidirectional streaming
    def InteractingHello(self, request_iterator, context):
        for request in request_iterator:
            print("InteractingHello Request Made:")
            print(request)

            hello_reply = greet_pb2.HelloReply()
            hello_reply.message = f"{request.greeting} {request.name}"

            yield hello_reply

def serve(): # actual server
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    #server needs to know where to direct the reqs, calling GreeterServicer calls the class above
    #no specific reason for 10 workers given
    greet_pb2_grpc.add_GreeterServicer_to_server(GreeterServicer(), server)
    server.add_insecure_port("localhost:50051") # add port so we can call and run grpc server
    server.start()
    server.wait_for_termination() # I think this means code won't stop until we do crtl + c

if __name__ == "__main__":
    serve()