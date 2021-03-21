//
//  AppDelegate.swift
//  BLEProofCentral
//
//  Created by Alexander Lavrushko on 22/03/2021.
//

import UIKit

@main
class AppDelegate: UIResponder, UIApplicationDelegate {

    var window: UIWindow?

    func application(_ application: UIApplication,
                     didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?) -> Bool {

        window = UIWindow(frame: UIScreen.main.bounds)

        let storyboard = UIStoryboard(name: "Main", bundle: nil)
        let rootVC = storyboard.instantiateInitialViewController() as? BLECentralViewController
        window?.rootViewController = rootVC
        window?.makeKeyAndVisible()

        return true
    }
}
