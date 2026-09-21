
/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// Copyright (c) 1996-2020, Live Networks, Inc.  All rights reserved
// A demo application, showing how to create and run a RTSP client (that can potentially receive multiple streams concurrently).
//
// NOTE: This code - although it builds a running application - is intended only to illustrate how to develop your own RTSP
// client application.  For a full-featured RTSP client application - with much more functionality, and many options - see
// "openRTSP": http://www.live555.com/openRTSP/

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "string"
#include "iostream"
#define _20240417

#ifdef __cplusplus
extern "C"
{
#endif

#include "ak_mem.h"
#include "ak_thread.h"
// #include "queue.h"
// #include "os_sys_api.h"
// #include "video_decodec.h"
/// #include "video_package.h"
#include "audio_decode.h"
#include "tuya_ring_buffer.h"
#include "tuya_ipc_p2p.h"
#include "video_decode.h"
#include "file_api.h"
#include "tuya_ipc_stream_storage.h"
#include "tuya_sdk.h"
#include "leo_api.h"
    extern int h264_is_keyframe(const unsigned char *buffer, int len);
    extern bool video_decode_push(char type, unsigned char *data, int len);
    extern bool video_decode_queue_reset(void);
    extern unsigned long long os_get_ms(void);
    extern bool video_record_data_push(record_data_node *node);
#ifdef __cplusplus
}
#endif
#if 0
#include "leo_VideoInput.hpp"
#include "leo_AudioDecodec.hpp"
#include "leo_AudioOutput.hpp"
#endif
// Forward function definitions:

// RTSP 'response handlers':
void continueAfterDESCRIBE(RTSPClient *rtspClient, int resultCode, char *resultString);
void continueAfterSETUP(RTSPClient *rtspClient, int resultCode, char *resultString);
void continueAfterPLAY(RTSPClient *rtspClient, int resultCode, char *resultString);

// Other event handler functions:
void subsessionAfterPlaying(void *clientData); // called when a stream's subsession (e.g., audio or video substream) ends
void subsessionByeHandler(void *clientData, char const *reason);
// called when a RTCP "BYE" is received for a subsession
void streamTimerHandler(void *clientData);
// called at the end of a stream's expected duration (if the stream has not already signaled its end using a RTCP "BYE")

// The main streaming routine (for each "rtsp://" URL):
void openURL(UsageEnvironment &env, char const *progName, char const *rtspURL);

// Used to iterate through each stream's 'subsessions', setting up each one:
void setupNextSubsession(RTSPClient *rtspClient);

// Used to shut down and close a stream (including its "RTSPClient" object):
void shutdownStream(RTSPClient *rtspClient, int exitCode = 1);

// A function that outputs a string that identifies each stream (for debugging output).  Modify this if you wish:
UsageEnvironment &operator<<(UsageEnvironment &env, const RTSPClient &rtspClient)
{
    return env << "[URL:\"" << rtspClient.url() << "\"]: ";
}

// A function that outputs a string that identifies each subsession (for debugging output).  Modify this if you wish:
UsageEnvironment &operator<<(UsageEnvironment &env, const MediaSubsession &subsession)
{
    return env << subsession.mediumName() << "/" << subsession.codecName();
}

void usage(UsageEnvironment &env, char const *progName)
{
    env << "Usage: " << progName << " <rtsp-url-1> ... <rtsp-url-N>\n";
    env << "\t(where each <rtsp-url-i> is a \"rtsp://\" URL)\n";
}

static RTSPClient *OurRtspClient = NULL;
static bool is_parse_sps_pps = false;

static char rtsp_stream_run_flag = 1;
static bool rtsp_stream_thread_flag = false;
static char rtsp_url_addres[128];

static int rtsp_stream_width = 0;
static int rtsp_stream_high = 0;
static ak_pthread_t rtsp_stream_thread_id = 0;

extern "C" int rtsp_stream_w_get(void)
{
    return rtsp_stream_width;
}
extern "C" int rtsp_stream_h_get(void)
{
    return rtsp_stream_high;
}
static void *live555MediaClient_main(void *arg)
{
    printf("live555MediaClient_main =========================================>>>> \n\r");
    rtsp_stream_high = rtsp_stream_width = 0;
    rtsp_stream_thread_flag = true;
    // Begin by setting up our usage environment:
    TaskScheduler *scheduler = BasicTaskScheduler::createNew();
    UsageEnvironment *env = BasicUsageEnvironment::createNew(*scheduler);
    is_parse_sps_pps = false;

    openURL(*env, NULL, rtsp_url_addres);

    OutPacketBuffer::maxSize = 90000; // 90000;
    // All subsequent activity takes place within the event loop:
    env->taskScheduler().doEventLoop(&rtsp_stream_run_flag); // does not return
    if (OurRtspClient != NULL)
    {

        shutdownStream(OurRtspClient);
    }
    rtsp_stream_thread_id = 0;
    rtsp_stream_thread_flag = false;
    ak_thread_exit();
    return NULL;
}

void live555SendSPSandPPS(void)
{
    is_parse_sps_pps = false;
}

extern "C" bool rtsp_stream_open(char *url)
{
    if (rtsp_stream_thread_flag)
    {
        printf("stream is open \n\r");
        return false;
    }

    if (!rtsp_stream_run_flag)
    {
        printf("stream is open \n\r");
        return false;
    }
#if 1
    // video_decode_api.open();
    // audio_decode_api.open(AUDIO_CHANNEL_MONO,AK_AUDIO_SAMPLE_RATE_8000,20,6);
#else
#if 0
	video_package_api.send_open("192.168.38.2",4561);
#else
    video_decode_api.open();
    video_package_api.receive_open("192.168.38.2", 4561);
    return true;
#endif
#endif

    memset(rtsp_url_addres, 0, sizeof(rtsp_url_addres));
    strcpy(rtsp_url_addres, url);

    rtsp_stream_run_flag = 0;
    ak_thread_create(&rtsp_stream_thread_id, live555MediaClient_main, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
    #ifndef _20240417
    ak_thread_detach(rtsp_stream_thread_id);
    #endif
    return true;
}

extern "C" bool rtsp_stream_close(void)
{

    if (rtsp_stream_thread_flag == 0)
    {
        printf("stream is close \n\r");
        return false;
    }

    rtsp_stream_run_flag = 1;
#ifdef _20240417
    ak_thread_join(rtsp_stream_thread_id);
    rtsp_stream_thread_id = 0;
#endif
    // tuya_ipc_ss_stop_event();
    // video_package_api.send_close();

    //	video_decode_api.close();
    // audio_decode_api.close();
    printf("%s=========================>>>\n", __func__);
    return true;
}
extern "C" bool is_rtsp_stream_run(void)
{

    return rtsp_stream_run_flag ? false : true;
}

// Define a class to hold per-stream state that we maintain throughout each stream's lifetime:

class StreamClientState
{
public:
    StreamClientState();
    virtual ~StreamClientState();

public:
    MediaSubsessionIterator *iter;
    MediaSession *session;
    MediaSubsession *subsession;
    TaskToken streamTimerTask;
    double duration;
};

// If you're streaming just a single stream (i.e., just from a single URL, once), then you can define and use just a single
// "StreamClientState" structure, as a global variable in your application.  However, because - in this demo application - we're
// showing how to play multiple streams, concurrently, we can't do that.  Instead, we have to have a separate "StreamClientState"
// structure for each "RTSPClient".  To do this, we subclass "RTSPClient", and add a "StreamClientState" field to the subclass:

class ourRTSPClient : public RTSPClient
{
public:
    static ourRTSPClient *createNew(UsageEnvironment &env, char const *rtspURL,
                                    int verbosityLevel = 0,
                                    char const *applicationName = NULL,
                                    portNumBits tunnelOverHTTPPortNum = 0);

protected:
    ourRTSPClient(UsageEnvironment &env, char const *rtspURL, int verbosityLevel, char const *applicationName, portNumBits tunnelOverHTTPPortNum);
    // called only by createNew();
    virtual ~ourRTSPClient();

public:
    StreamClientState scs;
};

// Define a data sink (a subclass of "MediaSink") to receive the data for each subsession (i.e., each audio or video 'substream').
// In practice, this might be a class (or a chain of classes) that decodes and then renders the incoming audio or video.
// Or it might be a "FileSink", for outputting the received data into a file (as is done by the "openRTSP" application).
// In this example code, however, we define a simple 'dummy' sink that receives incoming data, but does nothing with it.

class DummySink : public MediaSink
{
public:
    static DummySink *createNew(UsageEnvironment &env,
                                MediaSubsession &subsession,  // identifies the kind of data that's being received
                                char const *streamId = NULL); // identifies the stream itself (optional)

private:
    DummySink(UsageEnvironment &env, MediaSubsession &subsession, char const *streamId);
    // called only by "createNew()"
    virtual ~DummySink();

    static void afterGettingFrame(void *clientData, unsigned frameSize,
                                  unsigned numTruncatedBytes,
                                  struct timeval presentationTime,
                                  unsigned durationInMicroseconds);
    void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
                           struct timeval presentationTime, unsigned durationInMicroseconds);

private:
    // redefined virtual functions:
    virtual Boolean continuePlaying();

private:
    u_int8_t *fReceiveBuffer;
    MediaSubsession &fSubsession;
    char *fStreamId;
};

#define RTSP_CLIENT_VERBOSITY_LEVEL 1 // by default, print verbose output from each "RTSPClient"

static unsigned rtspClientCount = 0; // Counts how many streams (i.e., "RTSPClient"s) are currently in use.

void openURL(UsageEnvironment &env, char const *progName, char const *rtspURL)
{
    // Begin by creating a "RTSPClient" object.  Note that there is a separate "RTSPClient" object for each stream that we wish
    // to receive (even if more than stream uses the same "rtsp://" URL).
    RTSPClient *rtspClient = ourRTSPClient::createNew(env, rtspURL, RTSP_CLIENT_VERBOSITY_LEVEL, progName);
    if (rtspClient == NULL)
    {
        env << "Failed to create a RTSP client for URL \"" << rtspURL << "\": " << env.getResultMsg() << "\n";
        return;
    }

    ++rtspClientCount;
    OurRtspClient = rtspClient;

    // Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
    // Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
    // Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:
    rtspClient->sendDescribeCommand(continueAfterDESCRIBE);
}

// Implementation of the RTSP 'response handlers':
typedef void (*RTSP_Callback)(bool arg);
static RTSP_Callback openURL_Callback = NULL;
extern "C" void register_rtsp_callback(void (*backfunc)(bool))
{
    openURL_Callback = backfunc;
}

void continueAfterDESCRIBE(RTSPClient *rtspClient, int resultCode, char *resultString)
{
    do
    {
        UsageEnvironment &env = rtspClient->envir();                 // alias
        StreamClientState &scs = ((ourRTSPClient *)rtspClient)->scs; // alias

        if (resultCode != 0)
        {
            env << *rtspClient << "Failed to get a SDP description: " << resultString << "\n";
            delete[] resultString;
            if (openURL_Callback)
            {
                openURL_Callback(false);
            }
            break;
        }

        char *const sdpDescription = resultString;
        env << *rtspClient << "Got a SDP description:\n"
            << sdpDescription << "\n";

        // Create a media session object from this SDP description:
        scs.session = MediaSession::createNew(env, sdpDescription);
        delete[] sdpDescription; // because we don't need it anymore
        if (scs.session == NULL)
        {
            if (openURL_Callback)
            {
                openURL_Callback(false);
            }
            env << *rtspClient << "Failed to create a MediaSession object from the SDP description: " << env.getResultMsg() << "\n";
            break;
        }
        else if (!scs.session->hasSubsessions())
        {
            if (openURL_Callback)
            {
                openURL_Callback(false);
            }
            env << *rtspClient << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
            break;
        }

        // Then, create and set up our data source objects for the session.  We do this by iterating over the session's 'subsessions',
        // calling "MediaSubsession::initiate()", and then sending a RTSP "SETUP" command, on each one.
        // (Each 'subsession' will have its own data source.)
        scs.iter = new MediaSubsessionIterator(*scs.session);
        setupNextSubsession(rtspClient);

        if (openURL_Callback)
        {
            openURL_Callback(true);
        }
        printf("BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBb\n");
        return;
    } while (0);

    // An unrecoverable error occurred with this stream.
    shutdownStream(rtspClient);
}

// By default, we request that the server stream its data using RTP/UDP.
// If, instead, you want to request that the server stream via RTP-over-TCP, change the following to True:
#define REQUEST_STREAMING_OVER_TCP True // False//True

void setupNextSubsession(RTSPClient *rtspClient)
{
    UsageEnvironment &env = rtspClient->envir();                 // alias
    StreamClientState &scs = ((ourRTSPClient *)rtspClient)->scs; // alias

    scs.subsession = scs.iter->next();
    if (scs.subsession != NULL)
    {
        if (!scs.subsession->initiate())
        {
            env << *rtspClient << "Failed to initiate the \"" << *scs.subsession << "\" subsession: " << env.getResultMsg() << "\n";
            setupNextSubsession(rtspClient); // give up on this subsession; go to the next one
        }
        else
        {
            env << *rtspClient << "Initiated the \"" << *scs.subsession << "\" subsession (";
            if (scs.subsession->rtcpIsMuxed())
            {
                env << "client port " << scs.subsession->clientPortNum();
            }
            else
            {
                env << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum() + 1;
            }
            env << ")\n";

            // Continue setting up this subsession, by sending a RTSP "SETUP" command:
            rtspClient->sendSetupCommand(*scs.subsession, continueAfterSETUP, False, REQUEST_STREAMING_OVER_TCP);
        }
        return;
    }

    // We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY" command to start the streaming:
    if (scs.session->absStartTime() != NULL)
    {
        // Special case: The stream is indexed by 'absolute' time, so send an appropriate "PLAY" command:
        rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY, scs.session->absStartTime(), scs.session->absEndTime());
    }
    else
    {
        scs.duration = scs.session->playEndTime() - scs.session->playStartTime();
        rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY);
    }
}

void continueAfterSETUP(RTSPClient *rtspClient, int resultCode, char *resultString)
{
    do
    {
        UsageEnvironment &env = rtspClient->envir();                 // alias
        StreamClientState &scs = ((ourRTSPClient *)rtspClient)->scs; // alias

        if (resultCode != 0)
        {
            env << *rtspClient << "Failed to set up the \"" << *scs.subsession << "\" subsession: " << resultString << "\n";
            break;
        }

        env << *rtspClient << "Set up the \"" << *scs.subsession << "\" subsession (";
        if (scs.subsession->rtcpIsMuxed())
        {
            env << "client port " << scs.subsession->clientPortNum();
        }
        else
        {
            env << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum() + 1;
        }
        env << ")\n";

        // Having successfully setup the subsession, create a data sink for it, and call "startPlaying()" on it.
        // (This will prepare the data sink to receive data; the actual flow of data from the client won't start happening until later,
        // after we've sent a RTSP "PLAY" command.)

        scs.subsession->sink = DummySink::createNew(env, *scs.subsession, rtspClient->url());
        // perhaps use your own custom "MediaSink" subclass instead
        if (scs.subsession->sink == NULL)
        {
            env << *rtspClient << "Failed to create a data sink for the \"" << *scs.subsession
                << "\" subsession: " << env.getResultMsg() << "\n";
            break;
        }

        env << *rtspClient << "Created a data sink for the \"" << *scs.subsession << "\" subsession\n";
        scs.subsession->miscPtr = rtspClient; // a hack to let subsession handler functions get the "RTSPClient" from the subsession
        scs.subsession->sink->startPlaying(*(scs.subsession->readSource()),
                                           subsessionAfterPlaying, scs.subsession);
        // Also set a handler to be called if a RTCP "BYE" arrives for this subsession:
        if (scs.subsession->rtcpInstance() != NULL)
        {
            scs.subsession->rtcpInstance()->setByeWithReasonHandler(subsessionByeHandler, scs.subsession);
        }
    } while (0);
    delete[] resultString;

    // Set up the next subsession, if any:
    setupNextSubsession(rtspClient);
}

void continueAfterPLAY(RTSPClient *rtspClient, int resultCode, char *resultString)
{
    Boolean success = False;

    do
    {
        UsageEnvironment &env = rtspClient->envir();                 // alias
        StreamClientState &scs = ((ourRTSPClient *)rtspClient)->scs; // alias

        if (resultCode != 0)
        {
            env << *rtspClient << "Failed to start playing session: " << resultString << "\n";
            break;
        }

        // Set a timer to be handled at the end of the stream's expected duration (if the stream does not already signal its end
        // using a RTCP "BYE").  This is optional.  If, instead, you want to keep the stream active - e.g., so you can later
        // 'seek' back within it and do another RTSP "PLAY" - then you can omit this code.
        // (Alternatively, if you don't want to receive the entire stream, you could set this timer for some shorter value.)
        if (scs.duration > 0)
        {
            unsigned const delaySlop = 2; // number of seconds extra to delay, after the stream's expected duration.  (This is optional.)
            scs.duration += delaySlop;
            unsigned uSecsToDelay = (unsigned)(scs.duration * 1000000);
            scs.streamTimerTask = env.taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc *)streamTimerHandler, rtspClient);
        }

        env << *rtspClient << "Started playing session";
        if (scs.duration > 0)
        {
            env << " (for up to " << scs.duration << " seconds)";
        }
        env << "...\n";

        success = True;
    } while (0);
    delete[] resultString;

    if (!success)
    {
        // An unrecoverable error occurred with this stream.
        shutdownStream(rtspClient);
    }
}

// Implementation of the other event handlers:

void subsessionAfterPlaying(void *clientData)
{
    MediaSubsession *subsession = (MediaSubsession *)clientData;
    RTSPClient *rtspClient = (RTSPClient *)(subsession->miscPtr);

    // Begin by closing this subsession's stream:
    Medium::close(subsession->sink);
    subsession->sink = NULL;

    // Next, check whether *all* subsessions' streams have now been closed:
    MediaSession &session = subsession->parentSession();
    MediaSubsessionIterator iter(session);
    while ((subsession = iter.next()) != NULL)
    {
        if (subsession->sink != NULL)
            return; // this subsession is still active
    }

    // All subsessions' streams have now been closed, so shutdown the client:
    shutdownStream(rtspClient);
}

void subsessionByeHandler(void *clientData, char const *reason)
{
    MediaSubsession *subsession = (MediaSubsession *)clientData;
    RTSPClient *rtspClient = (RTSPClient *)subsession->miscPtr;
    UsageEnvironment &env = rtspClient->envir(); // alias

    env << *rtspClient << "Received RTCP \"BYE\"";
    if (reason != NULL)
    {
        env << " (reason:\"" << reason << "\")";
        delete[] (char *)reason;
    }
    env << " on \"" << *subsession << "\" subsession\n";

    // Now act as if the subsession had closed:
    subsessionAfterPlaying(subsession);
}

void streamTimerHandler(void *clientData)
{
    ourRTSPClient *rtspClient = (ourRTSPClient *)clientData;
    StreamClientState &scs = rtspClient->scs; // alias

    scs.streamTimerTask = NULL;

    // Shut down the stream:
    shutdownStream(rtspClient);
}

void shutdownStream(RTSPClient *rtspClient, int exitCode)
{
    UsageEnvironment &env = rtspClient->envir();                 // alias
    StreamClientState &scs = ((ourRTSPClient *)rtspClient)->scs; // alias

    // First, check whether any subsessions have still to be closed:
    if (scs.session != NULL)
    {
        Boolean someSubsessionsWereActive = False;
        MediaSubsessionIterator iter(*scs.session);
        MediaSubsession *subsession;

        while ((subsession = iter.next()) != NULL)
        {
            if (subsession->sink != NULL)
            {
                Medium::close(subsession->sink);
                subsession->sink = NULL;

                if (subsession->rtcpInstance() != NULL)
                {
                    subsession->rtcpInstance()->setByeHandler(NULL, NULL); // in case the server sends a RTCP "BYE" while handling "TEARDOWN"
                }

                someSubsessionsWereActive = True;
            }
        }

        if (someSubsessionsWereActive)
        {
            // Send a RTSP "TEARDOWN" command, to tell the server to shutdown the stream.
            // Don't bother handling the response to the "TEARDOWN".
            rtspClient->sendTeardownCommand(*scs.session, NULL);
        }
    }

    env << *rtspClient << "Closing the stream.\n";
    Medium::close(rtspClient);
    // Note that this will also cause this stream's "StreamClientState" structure to get reclaimed.

    if (--rtspClientCount == 0)
    {
        OurRtspClient = NULL;
        is_parse_sps_pps = false;
        // The final stream has ended, so exit the application now.
        // (Of course, if you're embedding this code into your own application, you might want to comment this out,
        // and replace it with "eventLoopWatchVariable = 1;", so that we leave the LIVE555 event loop, and continue running "main()".)
        // exit(exitCode);
    }
}

// Implementation of "ourRTSPClient":

ourRTSPClient *ourRTSPClient::createNew(UsageEnvironment &env, char const *rtspURL,
                                        int verbosityLevel, char const *applicationName, portNumBits tunnelOverHTTPPortNum)
{
    return new ourRTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum);
}

ourRTSPClient::ourRTSPClient(UsageEnvironment &env, char const *rtspURL,
                             int verbosityLevel, char const *applicationName, portNumBits tunnelOverHTTPPortNum)
    : RTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, -1)
{
}

ourRTSPClient::~ourRTSPClient()
{
}

// Implementation of "StreamClientState":

StreamClientState::StreamClientState()
    : iter(NULL), session(NULL), subsession(NULL), streamTimerTask(NULL), duration(0.0)
{
}

StreamClientState::~StreamClientState()
{
    delete iter;
    if (session != NULL)
    {
        // We also need to delete "session", and unschedule "streamTimerTask" (if set)
        UsageEnvironment &env = session->envir(); // alias

        env.taskScheduler().unscheduleDelayedTask(streamTimerTask);
        Medium::close(session);
    }
}

// Implementation of "DummySink":

// Even though we're not going to be doing anything with the incoming data, we still need to receive it.
// Define the size of the buffer that we'll use:
#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 5 * 1024 * 1024 // 5*1024*1024

DummySink *DummySink::createNew(UsageEnvironment &env, MediaSubsession &subsession, char const *streamId)
{
    return new DummySink(env, subsession, streamId);
}

DummySink::DummySink(UsageEnvironment &env, MediaSubsession &subsession, char const *streamId)
    : MediaSink(env),
      fSubsession(subsession)
{
    fStreamId = strDup(streamId);
    fReceiveBuffer = new u_int8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE];
}

DummySink::~DummySink()
{
    delete[] fReceiveBuffer;
    delete[] fStreamId;
}

void DummySink::afterGettingFrame(void *clientData, unsigned frameSize, unsigned numTruncatedBytes,
                                  struct timeval presentationTime, unsigned durationInMicroseconds)
{
    DummySink *sink = (DummySink *)clientData;
    sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

// If you don't want to see debugging output for each received frame, then comment out the following line:
// #define DEBUG_PRINT_EACH_RECEIVED_FRAME 1

#include "video_decode.h"
static void live555_rtsp_stream_insert(unsigned char *data, int len)
{
    // if((u > 48 ))
    // {
    //     return;
    // }
    unsigned char head[4] = {0x00, 0x00, 0x00, 0x01};
    record_data_node node;
    node.len = len + 4;
    node.is_video = true;

    node.data = (unsigned char *)ak_mem_alloc(MODULE_ID_VDEC, node.len);
    memcpy(node.data, head, sizeof(head));
    memcpy(&node.data[4], data, len);

    // if(rtsp_stream_thread_flag == false && video_decode_data_status())
    // {
    // printf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA:%d = = %d\n",get_video_decode_state(),video_decode_data_status());
    //     rtsp_stream_thread_flag = true;
    // }

    if (/* get_video_decode_state() == true && rtsp_stream_thread_flag &&  */ tuya_event_state_get() != TRANS_LIVE_VIDEO_START)
    {
        //  printf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA:%d = = %d\n",get_video_decode_state(),node.len);
        video_decode_push(0, (unsigned char *)node.data, node.len);
        video_record_data_push(&node);
    }

    if ((is_sdcard_insert() == true) && (tuya_ipc_ss_get_status() != E_STORAGE_ONGOING) && tuya_ipc_register_status_get() == E_IPC_ACTIVEATED)
    {
        // printf("==================================================>>>>>:%d \n",tuya_ipc_ss_get_status() );
        // tuya_ipc_ss_start_event();
    }
    else if ((is_sdcard_insert() == false) && (tuya_ipc_ss_get_status() == E_STORAGE_ONGOING))
    {
        // printf("==================================================>>>>>:%d \n",tuya_ipc_ss_get_status() );
        // tuya_ipc_ss_stop_event();
    }

    if (tuya_ipc_register_status_get() == E_IPC_ACTIVEATED)
    {
        tuya_ipc_ring_buffer_append_data(E_CHANNEL_VIDEO_MAIN, (unsigned char *)node.data, node.len, h264_is_keyframe((const unsigned char *)(node.data + 4), node.len - 4) ? E_VIDEO_I_FRAME : E_VIDEO_PB_FRAME, os_get_ms());

        unsigned char audio_frame[1280] = {0};
        tuya_ipc_ring_buffer_append_data(E_CHANNEL_AUDIO, (unsigned char *)audio_frame, 320, E_AUDIO_FRAME, os_get_ms());
    }

    ak_mem_free(node.data);
}

void DummySink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
                                  struct timeval presentationTime, unsigned /*durationInMicroseconds*/)
{
    // We've just received a frame of data.  (Optionally) print out information about it:
#ifdef DEBUG_PRINT_EACH_RECEIVED_FRAME
    if (fStreamId != NULL)
        envir() << "Stream \"" << fStreamId << "\"; ";
    envir() << fSubsession.mediumName() << "/" << fSubsession.codecName() << ":\tReceived " << frameSize << " bytes";
    if (numTruncatedBytes > 0)
        envir() << " (with " << numTruncatedBytes << " bytes truncated)";
    char uSecsStr[6 + 1]; // used to output the 'microseconds' part of the presentation time
    sprintf(uSecsStr, "%06u", (unsigned)presentationTime.tv_usec);
    envir() << ".\tPresentation time: " << (int)presentationTime.tv_sec << "." << uSecsStr;
    if (fSubsession.rtpSource() != NULL && !fSubsession.rtpSource()->hasBeenSynchronizedUsingRTCP())
    {
        envir() << "!"; // mark the debugging output to indicate that this presentation time is not RTCP-synchronized
    }
#ifdef DEBUG_PRINT_NPT
    envir() << "\tNPT: " << fSubsession.getNormalPlayTime(presentationTime);
#endif
    envir() << "\n";
#endif

    // printf("%s \n\r",fSubsession.codecName());
    if (strncmp(fSubsession.codecName(), "H264", 16) == 0)
    {
        // envir() << fSubsession.videoWidth() << "Width" << fSubsession.videoHeight() << "Height";

        // if(get_video_decode_state() == false)
        // {
        //     // printf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAa\n");

        //     rtsp_stream_width = fSubsession.videoWidth();
        //     rtsp_stream_high = fSubsession.videoHeight();
        //     // video_decode_open(0, 1920/* fSubsession.videoWidth() */, /* fSubsession.videoHeight() */1080);
        // }

        unsigned int num = 0;
        if (!is_parse_sps_pps)
        {

            SPropRecord *pSPropRecord = parseSPropParameterSets(fSubsession.fmtp_spropparametersets(), num);
            for (unsigned int i = 0; i < num; i++)
            {
                // video_package_api.send_write(pSPropRecord[i].sPropBytes,pSPropRecord[i].sPropLength);
                // video_decode_api.write(pSPropRecord[i].sPropBytes,pSPropRecord[i].sPropLength);
                live555_rtsp_stream_insert(pSPropRecord[i].sPropBytes, pSPropRecord[i].sPropLength);
            }

            delete[] pSPropRecord;
            is_parse_sps_pps = true;
        }
        live555_rtsp_stream_insert(fReceiveBuffer, frameSize);
        // printf("========================> recev:%d \n",frameSize);
        // video_package_api.send_write(fReceiveBuffer,frameSize);
        // video_decode_api.write(fReceiveBuffer,frameSize);
        // queue_data_push(QUEUE_CHANNEL_VIDEO_1, fReceiveBuffer, frameSize, os_get_ms(), ((fReceiveBuffer[0]&0x1F)== 0x05)?true:false);//(((fReceiveBuffer[0]&0x1F)== 0x02)||((fReceiveBuffer[0]&0x1F) == 0x07))
    }
    else if (strncmp(fSubsession.codecName(), "PCMU", 4) == 0)
    {
        // audio_decode_api.write((char*)fReceiveBuffer, frameSize);
        // printf("get audio frame size:%d \n\r",frameSize);
        // audio_output_write(fReceiveBuffer, frameSize);

        // printf("audio size:%d \n\r",frameSize);
#if 0
        static FILE* fp = nullptr;
        static int size = 0;
        if((size == 0) &&(fp == nullptr))
        {
            fp = fopen("/mnt/tf/pcmu","wb");
        }
        else
        {
            if(size < 300*1024)
            {
                fwrite(fReceiveBuffer,frameSize,1,fp);
                size+=frameSize;
            }
            else
            {
                fclose(fp);
                fp = nullptr;
            }
        }
#endif
    }

    continuePlaying();
}

Boolean DummySink::continuePlaying()
{
    if (fSource == NULL)
        return False; // sanity check (should not happen)

    // Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:
    fSource->getNextFrame(fReceiveBuffer, DUMMY_SINK_RECEIVE_BUFFER_SIZE,
                          afterGettingFrame, this,
                          onSourceClosure, this);
    return True;
}
