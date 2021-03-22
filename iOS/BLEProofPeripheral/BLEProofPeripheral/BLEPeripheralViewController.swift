//
//  ViewController.swift
//  BLEProofPeripheral
//
//  Created by Alexander Lavrushko on 22/03/2021.
//

import UIKit

class BLEPeripheralViewController: UIViewController {
    @IBOutlet weak var textViewStatus: UITextView!
    @IBOutlet weak var textViewLog: UITextView!
    @IBOutlet weak var textFieldAdvertisingData: UITextField!
    @IBOutlet weak var textFieldDataForRead: UITextField!
    @IBOutlet weak var textFieldDataForWrite: UITextField!
    @IBOutlet weak var textFieldDataForIndicate: UITextField!
    @IBOutlet weak var labelSubscribersCount: UILabel!
    @IBOutlet weak var switchAdvertising: UISwitch!

    override func viewDidLoad() {
        super.viewDidLoad()

        textViewStatus.layer.borderWidth = 1
        textViewStatus.layer.borderColor = UIColor.gray.cgColor
        textViewLog.layer.borderWidth = 1
        textViewLog.layer.borderColor = UIColor.gray.cgColor

        textFieldDataForWrite.isUserInteractionEnabled = false

        // text field delegate will hide keyboard after "return" is pressed
        textFieldAdvertisingData.delegate = self
        textFieldDataForRead.delegate = self
        textFieldDataForWrite.delegate = self
        textFieldDataForIndicate.delegate = self
    }

    @IBAction func onSwitchChangeAdvertising(_ sender: UISwitch) {
    }

    @IBAction func onTapSendIndication(_ sender: Any) {
    }

    @IBAction func onTapOpenSettings(_ sender: Any) {
    }

    @IBAction func onTapClearLog(_ sender: Any) {
    }
}

// MARK: - UI related methods
extension BLEPeripheralViewController: UITextFieldDelegate {
    func textFieldShouldReturn(_ textField: UITextField) -> Bool {
        textField.resignFirstResponder()
        return true
    }
}
