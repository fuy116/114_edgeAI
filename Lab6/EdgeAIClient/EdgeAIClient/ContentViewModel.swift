import Foundation
import Alamofire

public class ContentViewModel: ObservableObject {
    
    public static let url = "https://urufscarpvif.ap-northeast-1.clawcloudrun.com/mongo/last"
    
    @Published var records: [Record] = []
    
    private let timer: DispatchSourceTimer
    init(){
        timer = DispatchSource.makeTimerSource(flags: [], queue: DispatchQueue.global(qos: .userInitiated))
        timer.schedule(deadline: .now(), repeating: 1.0)
        timer.setEventHandler { [weak self] in
            self?.update()
        }
        timer.resume()
    }

    
    public func update(){
        let parameters: [String: Any] = [:]
        
        let request = AF.request(
            ContentViewModel.url,
            method: .get,
            parameters: parameters
        ).responseDecodable (of: LastRecordMessage.self){ [weak self] response in
            switch response.result {
            case .success(let message):
                DispatchQueue.main.async {
                    self?.records = [message.result]
                }
            case .failure(let error):
                print("Error \(error)")
            }
        }
    }
}
