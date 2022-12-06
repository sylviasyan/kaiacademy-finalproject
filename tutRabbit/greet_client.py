import greet_pb2_grpc
import greet_pb2
import time
import grpc
import pika


# need a function that will yield my different requests because I'm going to send multiple requests to the server
# before I receive a final reply. Useful if you wanna send data to server, have it collect things, then return a summarized response
def get_client_stream_requests():
    connection = pika.BlockingConnection(
    pika.ConnectionParameters(host='localhost'))
    channel = connection.channel()
    while True:
        rpcname = input("Please enter a name (or nothing to stop chatting): ")

        if rpcname == "":
            break
        rpcgreeting = input("Please enter a greeting (or nothing to stop chatting): ")
        
        
        channel.queue_declare(rpcgreeting)
        channel.queue_declare(rpcname)
        channel.basic_publish(exchange='', routing_key='password123', body=rpcgreeting)
        channel.basic_publish(exchange='', routing_key='password123', body=rpcname)
        
        
        hello_request = greet_pb2.HelloRequest(greeting = rpcgreeting, name = rpcname)
        yield hello_request
        # connection.close() 
        time.sleep(1)

# channel.queue_declare(rpcgreet)
#                 channel.queue_declare(rpcname)
#                 channel.basic_publish(exchange='', routing_key='password123', body=rpcgreet)
#                 channel.basic_publish(exchange='', routing_key='password123', body=rpcname)
                


# connection = pika.BlockingConnection(
#     pika.ConnectionParameters(host='localhost'))
# channel = connection.channel()

# channel.queue_declare(queue='hello')

# channel.basic_publish(exchange='', routing_key='hello', body='Hello World!')
# print(" [x] Sent 'Hello World!'")
# connection.close()

def run(): # runs the client
     # we want to create a channel we can connect our gRPC on
    with grpc.insecure_channel('localhost:5000') as channel: # allows us to make gRPC calls
        stub = greet_pb2_grpc.GreeterStub(channel)
        # we use the stub to call the gRPC calls

        connection = pika.BlockingConnection(
        pika.ConnectionParameters(host='localhost'))
        channel = connection.channel()

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

            channel.queue_declare(rpc_callGreet)
            channel.queue_declare(rpc_callName)
            # body=[rpc_callGreet,rpc_callName] having trouble converting any of the tutorial strings to string lists,
            # right now going to just try uploading them all individually (one for name one for greet)
            channel.basic_publish(exchange='', routing_key='password123', body=rpc_callGreet)
            channel.basic_publish(exchange='', routing_key='password123', body=rpc_callName)
            print(" [x] Sent " + rpc_callGreet + " and " + rpc_callName)
            connection.close()


            print("SayHello Response Received:")
            print(hello_reply)
        elif rpc_call == "2":
            rpc_callGreet = input("What is your greeting: ")
            rpc_callName = input("What is your name: ")
            hello_request = greet_pb2.HelloRequest(greeting = rpc_callGreet, name = rpc_callName)
            hello_replies = stub.ParrotSaysHello(hello_request)

            for hello_reply in hello_replies:
                channel.queue_declare(rpc_callGreet)
                channel.queue_declare(rpc_callName)
                channel.basic_publish(exchange='', routing_key='password123', body=rpc_callGreet)
                channel.basic_publish(exchange='', routing_key='password123', body=rpc_callName)
                print(" [x] Sent " + rpc_callGreet + " and " + rpc_callName)
                # connection.close()

                
                print("ParrotSaysHello Response Received:")
                print(hello_reply)
            connection.close() 
            # with connection close here, the client will see X sent name and greeting 3 times but 
            # the message will be name and greeting 1, name and greeting 2, then 3
            # right now this is because server-side streaming was looped 3 times and
            # I have no idea why she makes it loop so maybe these don't matter?
            # but in the server it'll show name: name, greeting: greeting once but will print
            # X received name, X received greeting, 3 times each
            # so not really sure the purpose of looping 3 times/why would we want to show why it's being incremented
            # rn all that's important is that I know I have access to how many times I print it,
            # I can talk to group and deicde how many times we'll have this print
        elif rpc_call == "3": # should print out response from server which will summarize the messages that I've sent
            delayed_reply = stub.ChattyClientSaysHello(get_client_stream_requests())
            print("ChattyClientSaysHello Response Received:")
            print(" [x] Sent: ")
            print(delayed_reply)
            connection.close() 

        elif rpc_call == "4":
            responses = stub.InteractingHello(get_client_stream_requests())

            for response in responses:
                print("InteractingHello Response Received: ")
                print(" [x] Sent: ")
                print(response)
            connection.close() 

if __name__ == "__main__":
    run()