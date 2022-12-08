import greet_pb2_grpc
import greet_pb2
import time
import grpc

# need a function that will yield my different requests because I'm going to send multiple requests to the server
# before I receive a final reply. Useful if you wanna send data to server, have it collect things, then return a summarized response
def get_client_stream_requests():
    while True:
        name = input("Please enter a name (or nothing to stop chatting): ")

        if name == "":
            break
        greeting = input("Please enter a greeting (or nothing to stop chatting): ")
        hello_request = greet_pb2.HelloRequest(greeting = greeting, name = name)
        yield hello_request
        time.sleep(1)


def run(): # runs the client
     # we want to create a channel we can connect our gRPC on
    with grpc.insecure_channel('localhost:1337') as channel: # allows us to make gRPC calls
        stub = greet_pb2_grpc.GreeterStub(channel)
        # we use the stub to call the gRPC calls
        print("1. SayHello - Unary")
        print("2. ParrotSaysHello - Server Side Streaming")
        print("3. ChattyClientSaysHello - Client Side Streaming")
        print("4. InteractingHello - Both Streaming")
        rpc_call = input("Which rpc would you like to make: ")
        # giving user option of what gRPC call they want to make

        if rpc_call == "1": # should print out a concatenation of the two strings
            rpc_callGreet = input("What is your greeting: ")
            rpc_callName = input("What is your name: ")
            hello_request = greet_pb2.HelloRequest(greeting = rpc_callGreet, name = rpc_callName)
            hello_reply = stub.SayHello(hello_request)
            print("SayHello Response Received:")
            print(hello_reply)
        elif rpc_call == "2":
            rpc_callGreet = input("What is your greeting: ")
            rpc_callName = input("What is your name: ")
            hello_request = greet_pb2.HelloRequest(greeting = rpc_callGreet, name = rpc_callName)
            hello_replies = stub.ParrotSaysHello(hello_request)

            for hello_reply in hello_replies:
                print("ParrotSaysHello Response Received:")
                print(hello_reply)
        elif rpc_call == "3": # should print out response from server which will summarize the messages that I've sent
            delayed_reply = stub.ChattyClientSaysHello(get_client_stream_requests())

            print("ChattyClientSaysHello Response Received:")
            print(delayed_reply)
        elif rpc_call == "4":
            responses = stub.InteractingHello(get_client_stream_requests())

            for response in responses:
                print("InteractingHello Response Received: ")
                print(response)

if __name__ == "__main__":
    run()