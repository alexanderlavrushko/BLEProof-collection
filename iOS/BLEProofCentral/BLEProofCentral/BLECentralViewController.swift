//
//  ViewController.swift
//  BLEProofCentral
//
//  Created by Alexander Lavrushko on 22/03/2021.
//

import UIKit

class BLECentralViewController: UIViewController {

    @IBOutlet weak var textViewStatus: UITextView!
    @IBOutlet weak var textViewLog: UITextView!
    @IBOutlet weak var switchConnect: UISwitch!
    @IBOutlet weak var textFieldDataForRead: UITextField!
    @IBOutlet weak var textFieldDataForWrite: UITextField!
    @IBOutlet weak var textFieldDataForIndicate: UITextField!
    @IBOutlet weak var labelSubscription: UILabel!

    override func viewDidLoad() {
        super.viewDidLoad()

        textViewStatus.layer.borderWidth = 1
        textViewStatus.layer.borderColor = UIColor.gray.cgColor
        textViewLog.layer.borderWidth = 1
        textViewLog.layer.borderColor = UIColor.gray.cgColor

        textFieldDataForRead.isUserInteractionEnabled = false
        textFieldDataForIndicate.isUserInteractionEnabled = false

        // text field delegate will hide keyboard after "return" is pressed
        textFieldDataForRead.delegate = self
        textFieldDataForWrite.delegate = self
        textFieldDataForIndicate.delegate = self
    }

    @IBAction func onChangeSwitchConnect(_ sender: UISwitch) {
    }

    @IBAction func onTapReadCharacteristic(_ sender: Any) {
    }

    @IBAction func onTapWriteCharacteristic(_ sender: Any) {
    }

    @IBAction func onTapOpenSettings(_ sender: Any) {
    }

    @IBAction func onTapClearLog(_ sender: Any) {
    }
}

// MARK: - UI related methods
extension BLECentralViewController: UITextFieldDelegate {
    func textFieldShouldReturn(_ textField: UITextField) -> Bool {
        textField.resignFirstResponder()
        return true
    }
}
