//
//  ContentView.swift
//  EdgeAIClient
//
//  Created by amk3y on 2025/6/2.
//

import SwiftUI
import SwiftData

struct ContentView: View {
    
    @StateObject var model: ContentViewModel = ContentViewModel()
    
    var body: some View {
        
        
        List {
            if let record = sortedRecords.first{
                Section {
                    VStack{
                        HStack {
                            VStack (alignment: .center) {
                                Text("電梯人數 (內)")
                                    .font(.headline)
                                Text("\(record.peopleInsideCount)")
                                    .font(.title)
                                    .bold()
                            }
                            Spacer()
                            VStack (alignment: .center) {
                                Text("電梯人數 (外)")
                                    .font(.headline)
                                Text("\(record.peopleOutsideCount)")
                                    .font(.title)
                                    .bold()
                            }
                        }
                    }
                    
                } header: {
                    
                } footer: {
                    Text("更新時間 \(record.timestamp)")
                }
            } else {
                Text("無紀錄")
            }
            
            Section {
                ForEach(sortedRecords){ record in
                    VStack (alignment: .leading){
                        Text("電梯編號 \(record.elevatorID)")
                        Text("電梯內人數 \(record.peopleInsideCount)")
                        Text("電梯外人數 \(record.peopleOutsideCount)")
                        Text("時間 \(record.timestamp)")
                    }
                }
            } header: {
                Text("歷史紀錄")
            }
        }
    }
    
    var sortedRecords: [Record] {
        model.records.sorted(by: {
            if ($0.date() != nil && $1.date() != nil){
                return $0.date()! > $1.date()!
            }else {
                return false
            }
        })
    }
}

#Preview {
    ContentView()
}
