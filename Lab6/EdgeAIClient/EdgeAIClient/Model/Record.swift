//
//  Record.swift
//  EdgeAIClient
//
//  Created by amk3y on 2025/6/2.
//

import Foundation

public struct Record: Codable, Identifiable {
    public let id: String
    let elevatorID: String
    let recordID: Int
    let peopleInsideCount: Int
    let peopleOutsideCount: Int
    let timestamp: String

    enum CodingKeys: String, CodingKey {
        case id = "_id"
        case elevatorID = "elevator_id"
        case recordID = "id"
        case peopleInsideCount = "people_inside_count"
        case peopleOutsideCount = "people_outside_count"
        case timestamp
    }

    func date() -> Date? {
        let formatter = DateFormatter()
        formatter.dateFormat = "yyyy/MM/dd HH:mm:ss"
        return formatter.date(from: timestamp)
    }
}
