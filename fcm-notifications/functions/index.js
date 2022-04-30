/**
 * Copyright 2016 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
'use strict';

const functions = require('firebase-functions');
const admin = require('firebase-admin');
admin.initializeApp();

/**
 * Triggers when a user gets a new follower and sends a notification.
 *
 * Followers add a flag to `/followers/{followedUid}/{followerUid}`.
 * Users save their device notification tokens to `/users/{followedUid}/notificationTokens/{notificationToken}`.
 */
exports.sendFollowerNotification = functions.database.ref('/devices/{devId}/state')
    .onWrite(async (change, context) => {
      const devId = context.params.devId;
      // If un-follow we exit the function.
      if (!change.after.val()) {
        return functions.logger.log(
          'User ',
          devId,
          'un-followed user',
          '1'
        );
      }
      functions.logger.log(
        'We have a new follower UID:',
        devId,
        'for user:',
        '1'
      );

      // Get the list of device notification tokens.
      const getDeviceTokensPromise = admin.database()
          .ref(`/notificationTokens`).once('value');

      // Get the follower profile.
      const getDeviceInfoPromise = admin.database()
          .ref('/devices/' + devId)

      // The snapshot to the user's tokens.
      let tokensSnapshot;

      // The array containing all the user's tokens.
      let tokens;

      const results = await Promise.all([getDeviceTokensPromise,getDeviceInfoPromise]);
      tokensSnapshot = results[0];
      const device = results[1];

      // Check if there are any device tokens.
      if (!tokensSnapshot.hasChildren()) {
        return functions.logger.log(
          'There are no notification tokens to send to.'
        );
      }
      functions.logger.log(
        'There are',
        tokensSnapshot.numChildren(),
        'tokens to send notifications to.'
      );
      functions.logger.log('Fetched device profile', device);

      // Notification details.
      const payload = {
        notification: {
          title: 'I²DS Messaging Service',
          body: `WARNING! ${device.hw} with ID ${device.self_id} detected an intruder.`,
          icon: "https://www.gstatic.com/devrel-devsite/prod/vcf74735f7c06cd017eb0bfd91ed83c965bdd5f0cbef3dc678f0e1f5f31be7e67/firebase/images/touchicon-180.png"
        }
      };

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
