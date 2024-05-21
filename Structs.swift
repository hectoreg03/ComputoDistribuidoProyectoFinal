//
//  Structs.swift
//  FinalComputo
//
//  Created by Marcelo Preciado Fausto on 19/05/24.
//

import Foundation

struct UserStruct: Decodable, Encodable{
    var inst : String = ""
    var user : String = ""
}

struct UserInfo: Decodable, Encodable{
    var user : String = ""
}

struct OnlineUserStruct: Decodable, Encodable{
    var inst  : String = ""
    var users : [User] = []
}

class OnlineUsers: ObservableObject{
    static    var shared  : OnlineUsers = OnlineUsers()
    @Published var users  : [User]    = []
    @Published var update : Bool        = false
}

struct JoinRequestStruct: Decodable, Encodable{
    var inst : String = ""
    var user : String = ""
    var room : ChatRoom = ChatRoom()

}
  

class ChatRoom: ObservableObject, Hashable, Codable{
    enum CodingKeys : CodingKey {
            case name
            case admin
            case users
    }
    
    func encode(to encoder: any Encoder) throws {
        var container = encoder.container(keyedBy: CodingKeys.self)
        try container.encode(name             , forKey: .name             )
        try container.encode(admin         , forKey: .admin         )
        try container.encode(users         , forKey: .users         )

    }
    
    required init(from decoder: any Decoder) throws {
        let container = try decoder.container(keyedBy: CodingKeys.self)
        name      = try container.decode(String.self, forKey: .name)
        admin = try container.decode(String.self  , forKey:.admin  )
        users = try container.decode([String].self  , forKey:.users  )


    }
    static func == (lhs: ChatRoom, rhs: ChatRoom) -> Bool {
         return lhs.id == rhs.id
     }
     
     func hash(into hasher: inout Hasher) {
         hasher.combine(id)
     }
    
    @Published var name : String = ""
    @Published var admin : String = ""
    @Published var users  : [String]    = []
    @Published var receavedMessages  : [MessageData]    = []

    var id: UUID = UUID()
    
    init(name: String, admin: String, users: [String]){
        self.name = name
        self.admin = admin
        self.users = users
    }
    
    init(name: String){
        self.name = name
    }
    init(){
        
    }
    
    func toJson() -> String {
        do {
            let jsonEncoder     = JSONEncoder()
            let jsonData        = try jsonEncoder.encode(self)
            if let json         = String(data: jsonData, encoding: String.Encoding.utf8) {
                return json
            } else {
                print("[ERROR CONVERTING DATA]")
            }
        } catch {
            print(error)
        }
        return ""
    }

}

class ChatRooms: ObservableObject, Codable{
    enum CodingKeys : CodingKey {
            case inst
            case chatRooms
    }
    
    func encode(to encoder: any Encoder) throws {
        var container = encoder.container(keyedBy: CodingKeys.self)
        try container.encode(inst             , forKey: .inst             )
        try container.encode(chatRooms         , forKey: .chatRooms         )

    }
    
    required init(from decoder: any Decoder) throws {
        let container = try decoder.container(keyedBy: CodingKeys.self)
        inst             = try container.decode(String.self   , forKey: .inst          )
        chatRooms         = try container.decode([ChatRoom].self  , forKey: .chatRooms  )

    }
    
    init(){
        
    }
    
    static    var shared  : ChatRooms = ChatRooms()
    @Published var inst  : String = ""
    @Published var chatRooms  : [ChatRoom]    = []
    @Published var update : Bool        = false
}

struct MessageData: Codable, Hashable{
    var inst : String = ""
    var destination : String = ""
    var target : String = ""
    var message : String = ""
    var sender : String = ""
    
    init(){
        
    }
    init(message: String, sender: String){
        self.message = message
        self.sender = sender
    }
}


class AlertManager: ObservableObject {
    static let shared = AlertManager()
    
    @Published var presentAlert  : Bool = false
    @Published var nameRequest   : String = ""
    @Published var groupRequest  : ChatRoom = ChatRoom()


}
