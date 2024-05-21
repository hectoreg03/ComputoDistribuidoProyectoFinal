//
//  TCPClient.swift
//  FinalComputo
//
//  Created by Marcelo Preciado Fausto on 19/05/24.
//

import Foundation
import Network

protocol TCPDelegate {
    func messageReady(message: String)
}


class TCPClient : ObservableObject{
    static     var shared       : TCPClient  = TCPClient()
    @Published var connected    : Bool      = false
    @Published var receivedMsg  : String    = ""
    var delegate        : TCPDelegate?
    
    var connection  : NWConnection?
    var host        : NWEndpoint.Host = NWEndpoint.Host("0.0.0.0")
    var port        : NWEndpoint.Port = NWEndpoint.Port("9008")!
    
    init(){
        
    }
    
    func initServer(hostName: String, port: Int) {
        self.host        = NWEndpoint.Host(hostName)
        self.port        = NWEndpoint.Port("\(port)")!
    }
    
    func start() {
        if !connected {
            print("[CONNECTION START TO: \(self.host):\(self.port)]")
            self.connection = NWConnection(host: self.host, port: self.port, using: .tcp)
            connected = false
            self.connection!.stateUpdateHandler = self.didChange(state:)
            self.startReceive()
            self.connection!.start(queue: .main)
        }
    }
    
    func stop() {
        self.send(json: "{\"inst\" : \"DISCONNECT\", \"user\": \"\(User.shared.user)\"}")
        connected = false
        print("[CONNECTION STOP]")
        self.connection!.cancel()
        User.shared.update.toggle()

    }
    
    func startReceive() {
        self.connection!.receive(minimumIncompleteLength: 1, maximumLength: 1000000) { data, _, isDone, error in
            if data != nil{
                let messageString = (String(decoding: data!, as: UTF8.self))
                self.delegate?.messageReady(message: messageString)
                
                self.receivedMsg = String(decoding: data!, as: UTF8.self)
            }else{
                self.receivedMsg = ""
            }
            
        }
    }
    
        
    func send(json: String) {
        if connected{
            let data = json.data(using: .utf8)!
            self.connection!.send(content: data, completion: NWConnection.SendCompletion.contentProcessed { error in
                if let error = error {
                    NSLog("[CONNECTION SEND ERROR] %@", "\(error)")
                } else {
                    print("Send Succesfully")
                }
            })
            startReceive()
        }

    }
    
    private func didChange(state: NWConnection.State) {
        switch state {
        case .setup:
            print("[SETUP CONNECTION]")
            connected = false
            break
        case .waiting(let error):
            print("[CONNECTION ERROR] \(error)")
            connected = false
        case .preparing:
            connected = false
            break
        case .ready:
            print("[CONNECTION READY]")
            User.shared.update.toggle()
            connected = true
            break
        case .failed(let error):
            print("[CONNECTION DID ERROR] \(error)")
            connected = false
            self.stop()
        case .cancelled:
            print("[CONNECTION CANCELED]")
            User.shared.update.toggle()
            self.send(json: "{\"inst\" : \"DISCONNECT\", \"user\": \"\(User.shared.user)\"}")
            
            self.stop()
        @unknown default:
            break
        }
    }
    
}
