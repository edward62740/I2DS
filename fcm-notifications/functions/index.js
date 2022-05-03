'use strict';
const functions = require('firebase-functions');
const admin = require('firebase-admin');
admin.initializeApp();

/**
 * Triggers when the state of a sensor changes.
 * Sends an alert message for S_ALERTING and a notification for other state changes.
 */
exports.I2DSstateChangeNotification = functions.region('asia-southeast1').database.ref('/devices/{devId}/state')
  .onWrite(async (change, context) => {
    const devId = (context.params.devId).replace(/\D/g, '');

    const getTokensPromise = admin.database()
      .ref(`/notificationTokens`).once('value');

    const getDeviceInfoPromise = admin.database()
      .ref(`/devices`).once('value');

    let tokensSnapshot;
    let deviceSnapshot;
    let tokens;
    let devices, levels;

    tokensSnapshot = await Promise.resolve(getTokensPromise);
    deviceSnapshot = await Promise.resolve(getDeviceInfoPromise);

    devices = Object.values(deviceSnapshot.val());

    let hw_string;
    switch (devices[devId].hw) {
      case 136:
        hw_string = 'CPN';
        break;
      case 137:
        hw_string = 'PIRSN';
        break;
      case 138:
        hw_string = 'ACSN';
        break;
      default:
        hw_string = 'Unknown device';
        break;
    }
    let state_string;
    switch (devices[devId].state) {
      case 5:
        state_string = 'been activated';
        break;
      case 6:
        state_string = 'been deactivated';
        break;
      case 202:
        state_string = 'detected a hardware fault';
        break;
      case 203:
        state_string = 'detected an operational fault';
        break;
      default:
        state_string = 'encountered an unknown error';
        break;
    }
    const warning_payload_pirsn = {
      notification: {
        title: 'I²DS Messaging Service',
        body: 'WARNING! ' + hw_string + ' (ID ' + devices[devId].self_id + ') has detected motion.',
      }
    };
    const warning_payload_acsn = {
      notification: {
        title: 'I²DS Messaging Service',
        body: 'WARNING! ' + hw_string + ' (ID ' + devices[devId].self_id + ') has detected that a door has been opened. Refer to the tagged location of this sensor.',
      }
    };
    const info_payload = {
      notification: {
        title: 'I²DS Messaging Service',
        body: '' + hw_string + ' (ID ' + devices[devId].self_id + ') has ' + state_string + '.',
        icon: 'https://github.com/edward62740/Wireless-Mesh-Network-System/blob/master/Documentation/ltsn.png',
      }
    };
    const priority = {
      android: {
        priority: 'high',
      }
    };

    // Listing all tokens as an array.
    tokens = Object.keys(tokensSnapshot.val());
    levels = Object.values(tokensSnapshot.val());
    const tokensToRemove = [];
    await Promise.all(tokens.map(async (token, index) => {
      let response = (devices[devId].state === 204) ?
        ((devices[devId].hw == 137) ? await admin.messaging().sendToDevice(token, warning_payload_pirsn, priority)
          : await admin.messaging().sendToDevice(token, warning_payload_acsn, priority))
        : (levels[index] == 1 || levels[index] == 3) ? await admin.messaging().sendToDevice(token, info_payload, priority) : null;
      const error = response.results[0].error;
      if (error) {
        functions.logger.error(
          'Failure sending notification to',
          token,
          error
        );
        // Cleanup the tokens who are not registered anymore.
        if (error.code === 'messaging/invalid-registration-token' ||
          error.code === 'messaging/registration-token-not-registered') {
          tokensToRemove.push(tokensSnapshot.ref.child(tokens[index]).remove());
        }
      }
    }));
    return Promise.all(tokensToRemove);
  });

/**
 * Triggers when a new sensor is added to the database.
 * Sends a message to indicate the new device.
 */
exports.I2DSdeviceAddedNotification = functions.region('asia-southeast1').database.ref('/devices/{devId}')
  .onCreate(async (change, context) => {
    const devId = (context.params.devId).replace(/\D/g, '');

    const getTokensPromise = admin.database()
      .ref(`/notificationTokens`).once('value');

    const getDeviceInfoPromise = admin.database()
      .ref(`/devices`).once('value');

    let tokensSnapshot;
    let deviceSnapshot;
    let tokens;
    let devices;

    tokensSnapshot = await Promise.resolve(getTokensPromise);
    deviceSnapshot = await Promise.resolve(getDeviceInfoPromise);

    devices = Object.values(deviceSnapshot.val());

    let hw_string;
    switch (devices[devId].hw) {
      case 136:
        hw_string = 'CPN';
        break;
      case 137:
        hw_string = 'PIRSN';
        break;
      case 138:
        hw_string = 'ACSN';
        break;
      default:
        hw_string = 'Unknown device';
        break;
    }

    const new_payload = {
      notification: {
        title: 'I²DS Messaging Service',
        body: '' + hw_string + ' (ID ' + devices[devId].self_id + ') has joined the system.',
      }
    };
    // Listing all tokens as an array.
    tokens = Object.keys(tokensSnapshot.val());
    // Send notifications to all tokens.
    const response = await admin.messaging().sendToDevice(tokens, new_payload);
    // For each message check if there was an error.
    const tokensToRemove = [];
    response.results.forEach((result, index) => {
      const error = result.error;
      if (error) {
        functions.logger.error(
          'Failure sending notification to',
          tokens[index],
          error
        );
        // Cleanup the tokens who are not registered anymore.
        if (error.code === 'messaging/invalid-registration-token' ||
          error.code === 'messaging/registration-token-not-registered') {
          tokensToRemove.push(tokensSnapshot.ref.child(tokens[index]).remove());
        }
      }
    });
    return Promise.all(tokensToRemove);
  });

/**
 * Triggers when the cpn info data changes.
 * Sends a message informing the user of either a PR or security failure.
 */
exports.I2DSinfoNotification = functions.region('asia-southeast1').database.ref('/info')
  .onUpdate(async (change, context) => {

    const getTokensPromise = admin.database()
      .ref(`/notificationTokens`).once('value');

    const getInfoPromise = admin.database()
      .ref(`/info`).once('value');

    let tokensSnapshot;
    let infoSnapshot;
    let tokens;
    let info;
    let levels;

    tokensSnapshot = await Promise.resolve(getTokensPromise);
    infoSnapshot = await Promise.resolve(getInfoPromise);

    info = Object.values(infoSnapshot.val());
    functions.logger.error(
      'INFO',
      info[0].pr,
      info[0].sec
    );
    let payload;
    if (info.pr == "false") {
      payload = {
        notification: {
          title: 'I²DS Messaging Service',
          body: 'Power failure detected.',
        }
      };
    }
    else if (info.sec == "false") {
      payload = {
        notification: {
          title: 'I²DS Messaging Service',
          body: 'Security breach detected.',
        }
      };
    }
    else  {
      payload = {
        notification: {
          title: 'I²DS Messaging Service',
          body: 'no.',
        }
      };
    }

    // Listing all tokens as an array.
    tokens = Object.keys(tokensSnapshot.val());
    levels = Object.values(tokensSnapshot.val());
    const tokensToRemove = [];
    await Promise.all(tokens.map(async (token, index) => {
      if (levels[index] == 0 || levels[index] == 2) {
        let response = await admin.messaging().sendToDevice(tokens, payload);
        const error = response.results[0].error;
        if (error) {
          functions.logger.error(
            'Failure sending notification to',
            token,
            error
          );
          // Cleanup the tokens who are not registered anymore.
          if (error.code === 'messaging/invalid-registration-token' ||
            error.code === 'messaging/registration-token-not-registered') {
            tokensToRemove.push(tokensSnapshot.ref.child(tokens[index]).remove());
          }
        }
      }

    }));
    return Promise.all(tokensToRemove);
  });
