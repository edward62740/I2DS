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
        hw_string = 'unknown';
        break;
    }
    let state_string;
    switch (devices[devId].state) {
      case 5:
        state_string = 'activated';
        break;
      case 6:
        state_string = 'deactivated';
        break;
      default:
        state_string = 'unknown';
        break;
    }
    const warning_payload = {
      notification: {
        title: 'I²DS Messaging Service',
        body: 'WARNING! ' + hw_string + ' (ID ' + devices[devId].self_id + ') detected an intruder.',
      }
    };
    const info_payload = {
      notification: {
        title: 'I²DS Messaging Service',
        body: '' + hw_string + ' (ID ' + devices[devId].self_id + ') has been ' + state_string + '.',
      }
    };
    const priority = {
      android: {
        priority: 'high',
      }
    };

    // Listing all tokens as an array.
    tokens = Object.keys(tokensSnapshot.val());
    // Send notifications to all tokens.
    const response = (devices[devId].state === 204) ? await admin.messaging().sendToDevice(tokens, warning_payload, priority) : await admin.messaging().sendToDevice(tokens, info_payload, priority);
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
        hw_string = 'unknown';
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

    tokensSnapshot = await Promise.resolve(getTokensPromise);
    infoSnapshot = await Promise.resolve(getInfoPromise);

    info = Object.values(infoSnapshot.val());
    let payload;
    if (info.pr === 'false') {
      payload = {
        notification: {
          title: 'I²DS Messaging Service',
          body: 'Power failure detected.',
        }
      };
    }
    else if (info.sec === 'false')
    {
      payload = {
        notification: {
          title: 'I²DS Messaging Service',
          body: 'Security breach detected.',
        }
      };
    }



    // Listing all tokens as an array.
    tokens = Object.keys(tokensSnapshot.val());
    // Send notifications to all tokens.
    const response = await admin.messaging().sendToDevice(tokens, payload);
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
