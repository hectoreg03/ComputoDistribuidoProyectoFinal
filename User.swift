//
//  User.swift
//  FinalComputo
//
//  Created by Marcelo Preciado Fausto on 19/05/24.
//

import Foundation

class User: ObservableObject, Codable, Hashable{
    static func == (lhs: User, rhs: User) -> Bool {
         return lhs.id == rhs.id
     }
     
     func hash(into hasher: inout Hasher) {
         hasher.combine(id)
     }
    
    
    enum CodingKeys : CodingKey {
            case user
            case password
            case isActive
    }
    
    func encode(to encoder: any Encoder) throws {
        var container = encoder.container(keyedBy: CodingKeys.self)
        try container.encode(user             , forKey: .user             )
        try container.encode(password         , forKey: .password         )
        try container.encode(isActive         , forKey: .isActive         )

    }
    
    required init(from decoder: any Decoder) throws {
        let container = try decoder.container(keyedBy: CodingKeys.self)
        user             = try container.decode(String.self   , forKey: .user          )
        password         = try container.decode(String.self  , forKey: .password      )
        isActive         = try container.decode(Bool.self  , forKey: .isActive      )

    }
    static var shared : User = User(user: "", password: "")
    @Published var update : Bool = false
    @Published var errorMessage : String = ""
    @Published var loginCorrect : Bool = false
    var id: UUID = UUID()
    @Published var chatting : String = ""
    @Published var user     : String = ""
    @Published var isActive : Bool = false
    @Published var password : String = ""
    @Published var adminRooms : [String] = []
    @Published var receavedMessages : [MessageData] = []



    init(user: String, password: String) {
        self.user = user
        self.password = password
    }
}
