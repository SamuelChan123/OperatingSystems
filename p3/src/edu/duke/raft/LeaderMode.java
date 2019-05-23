package edu.duke.raft;

import java.util.Timer;
import java.util.TimerTask;

public class LeaderMode extends RaftMode {

  protected final static int HEARTBEAT_INTERVAL_ID = 777;
  // protected int lastCommitted = -1;
  protected boolean outdated = false;
  protected Entry[] ent = new Entry[1];
  protected Timer t;
  protected int[] prevIndex = new int[mConfig.getNumServers()+1];
  // protected int[] matchIndex = new int[mConfig.getNumServers()+1];

  public void go () {
    synchronized (mLock) {
      int term = mConfig.getCurrentTerm();
      // RaftResponses.setTerm(term);
      System.out.println ("S" + 
			  mID + 
			  "." + 
			  term + 
			  ": switched to leader mode.");
      t = scheduleTimer(HEARTBEAT_INTERVAL, mID);
      RaftResponses.clearAppendResponses(term);
      ent = getLastEntry();
      int prev = mLog.getLastIndex() - 1;
      int logTerm = prev <= -1 ? -1 : mLog.getEntry(prev).term;
      // System.out.println("Leader S"+mID+"."+term+" last log index: "+mLog.getLastIndex()+" and last log term: "+mLog.getLastTerm());
      //Do we assume that every follower starts with at least one log entry?
      for (int i = 1; i <= mConfig.getNumServers(); i++) {
          if (i != mID) {
            // matchIndex[i] = mLog.getLastIndex()+1;
            // int prev = mLog.getLastIndex() == 0 ? mLog.getLastIndex() : mLog.getLastIndex() -1;
            // if (prev == 0) {
            //   ent == null;
            // }
            // System.out.println("Sending log entry to S"+i+": "+prev+" "+logTerm);
            prevIndex[i] = mLog.getLastIndex();
            remoteAppendEntries(i,mConfig.getCurrentTerm(),mID,prev,logTerm,ent,mCommitIndex);
          }
        }
    }
  }
  
  // @param candidate’s term
  // @param candidate requesting vote
  // @param index of candidate’s last log entry
  // @param term of candidate’s last log entry
  // @return 0, if server votes for candidate; otherwise, server's
  // current term
  public int requestVote (int candidateTerm,
			  int candidateID,
			  int lastLogIndex,
			  int lastLogTerm) {
    synchronized (mLock) {
      int term = mConfig.getCurrentTerm ();
      int vote = term;

      if (candidateTerm > term){
        // mConfig.setCurrentTerm(candidateTerm,0);
        // if (lastLogIndex >= mLog.getLastIndex() && lastLogTerm >= mLog.getLastTerm()){ //&& lastLogIndex >= mLog.getLastIndex() && lastLogTerm >= mLog.getLastTerm()) {
        // outdated = true;
        t.cancel();
        mConfig.setCurrentTerm(candidateTerm,candidateID);
        outdated = true;
        RaftServerImpl.setMode(new FollowerMode());
        // System.out.println(mConfig);
        return 0;
        // }
      }
      return vote;
    }
  }
  

  // @param leader’s term
  // @param current leader
  // @param index of log entry before entries to append
  // @param term of log entry before entries to append
  // @param entries to append (in order of 0 to append.length-1)
  // @param index of highest committed entry
  // @return 0, if server appended entries; otherwise, server's
  // current term
  public int appendEntries (int leaderTerm,
			    int leaderID,
			    int prevLogIndex,
			    int prevLogTerm,
			    Entry[] entries,
			    int leaderCommit) {
    synchronized (mLock) {
      int term = mConfig.getCurrentTerm ();
      int result = term;

      //If leader with higher term exists, switch to follower mode and roll back log to append the entries
      if (leaderTerm > term) {
        // mConfig.setCurrentTerm(leaderTerm,0);
        // if (prevLogIndex >= mLog.getLastIndex() && prevLogTerm >= mLog.getLastTerm()){//&& prevLogIndex >= mLog.getLastIndex() && prevLogTerm >= mLog.getLastTerm()){
        //outdated = true;
        // mLog.insert(entries,prevLogIndex,prevLogTerm);
        // mCommitIndex = leaderCommit;
        t.cancel();
        mConfig.setCurrentTerm(leaderTerm,0);
        term = mConfig.getCurrentTerm();
        outdated = true;
        RaftServerImpl.setMode(new FollowerMode());
        return term;
        // }
      }
      // else if (leaderTerm == term && prevLogIndex > mLog.getLastIndex() && prevLogTerm > mLog.getLastTerm()) {
      //   t.cancel();
      //   mConfig.setCurrentTerm(leaderTerm,0);
      //   term = mConfig.getCurrentTerm();
      //   RaftServerImpl.setMode(new FollowerMode());
      //   return term;
      // } 

      //Append client entries to my log
      // if (leaderID == mID && prevLogIndex == mLog.getLastIndex() && prevLogTerm == mLog.getLastTerm()) {
      //   // mLog.insert(entries,prevLogIndex, prevLogTerm);
      //   mLog.insert(entries,prevLogIndex,prevLogTerm);
      //   ent = entries;
      //   return 0;
      // }
      return result;
    }
  }

  // @param id of the timer that timed out
  public void handleTimeout (int timerID) {
    synchronized (mLock) {
      if (timerID == mID) {
        //Handle

        int term = mConfig.getCurrentTerm();

        // System.out.println("Leader S"+mID+"."+term+" log: "+mLog);

        t.cancel();
        // if (!outdated) {

        //Check if followers have approved the new entries in the log:
        int[] resp = RaftResponses.getAppendResponses(term);
        RaftResponses.clearAppendResponses(term);
        RaftResponses.clearVotes(term);
        RaftResponses.setTerm(mConfig.getCurrentTerm());
        // if (resp != null) {
        // System.out.println("Leader S"+mID+"."+term+" last log index: "+mLog.getLastIndex()+" and last log term: "+mLog.getLastTerm());
        int count = 1;
        int logTerm;
        if (resp != null) {
          for (int i = 1; i <= mConfig.getNumServers(); i++) {
            if (i != mID) {
              //check responses
              if (resp[i] > mConfig.getCurrentTerm()) {
                t.cancel();
                mConfig.setCurrentTerm(resp[i],0);
                outdated = true;
                RaftServerImpl.setMode(new FollowerMode());
                break;
              }
              else if (resp[i] == 0) {
                count++;
                prevIndex[i] = mLog.getLastIndex();
                // System.out.println("Response from S"+i+": "+resp[i]);
                ent = getLastEntry();
                // ent = null;
                int prev = mLog.getLastIndex() - 1;
                logTerm = prev == -1 ? -1 : mLog.getEntry(prev).term;
                // System.out.println("Prev entry to S"+i+" is: "+prev+" "+logTerm);
                remoteAppendEntries(i,mConfig.getCurrentTerm(),mID,prev,logTerm,ent,mCommitIndex);
              }
              else{ //log repair
                ent = getEntries(prevIndex[i]);
                // System.out.println("Sending log entry to S"+i+": "+ent);
                logTerm = (prevIndex[i]-1 < 0) ? -1 : mLog.getEntry(prevIndex[i]-1).term;
                // System.out.println("(Repair) Prev entry of S"+i+" is: (sub 1) "+prevIndex[i]+" "+logTerm);
                remoteAppendEntries(i,mConfig.getCurrentTerm(),mID,prevIndex[i]-1,logTerm,ent,mCommitIndex);
                if (prevIndex[i] > 0) {
                  prevIndex[i]--;
                }
              }
            }
           }
          } else { //Send heartbeats even if we can't count votes
            ent = null;
            int prev = mLog.getLastIndex() - 1;
            logTerm = prev <= -1 ? -1 : mLog.getEntry(prev).term;
            // System.out.println("Leader S"+mID+"."+term+" last log index: "+mLog.getLastIndex()+" and last log term: "+mLog.getLastTerm());
            //Do we assume that every follower starts with at least one log entry?
            for (int i = 1; i <= mConfig.getNumServers(); i++) {
                if (i != mID) {
                  // System.out.println("Sending log entry to S"+i+": "+prev+" "+logTerm);
                  remoteAppendEntries(i,mConfig.getCurrentTerm(),mID,prev,logTerm,ent,mCommitIndex);
                }
            }
          }

           //If a majority approved the append, commit and apply to the state machine
          if (!outdated) {
            if (count > mConfig.getNumServers()/2) {
            mCommitIndex = mLog.getLastIndex();
            // mLastApplied = mCommitIndex;
            }
          // }
            t = scheduleTimer(HEARTBEAT_INTERVAL, mID);
          }
          

         
        // }
        // else { //If current term overwritten, send heartbeats to indicate still alive
        //   ent = null;
        //   int prev = mLog.getLastIndex() - 1;
        //   int logTerm = prev <= -1 ? -1 : mLog.getEntry(prev).term;
        //   System.out.println("Leader S"+mID+"."+term+" last log index: "+mLog.getLastIndex()+" and last log term: "+mLog.getLastTerm());
        //   //Do we assume that every follower starts with at least one log entry?
        //   for (int i = 1; i <= mConfig.getNumServers(); i++) {
        //       if (i != mID) {
        //         System.out.println("Sending log entry to S"+i+": "+prev+" "+logTerm);
        //         remoteAppendEntries(i,mConfig.getCurrentTerm(),mID,prev,logTerm,ent,mCommitIndex);
        //       }
        //   }
        }
        else {
          throw new RuntimeException("Oops I didn't set this timer!");
        }
    }
  }

  public Entry[] getLastEntry() {
    Entry[] entry = new Entry[1];
    entry[0] = mLog.getEntry(mLog.getLastIndex());
    return entry;
  }

  //Retrieve all entries starting at specific index and return array of them
  public Entry[] getEntries(int startIndex) {
    int iter = startIndex;
    Entry[] entries = new Entry[mLog.getLastIndex()-startIndex+1];
    for (int i = 0; i <= (mLog.getLastIndex()-startIndex); i++) {
      entries[i] = mLog.getEntry(iter);
      iter++;
    }
    return entries;
  }

  // public void logRepair(int serverID) {
  //   //repair the log
  //   remoteAppendEntries(i,mConfig.getCurrentTerm(),mID,mLog.getLastIndex()-1,mLog.getEntry(mLog.getLastIndex()-1).term,ent,mCommitIndex);
  // }
}

          // protected final void remoteAppendEntries (final int serverID,
          //     final int leaderTerm,
          //     final int leaderID,
          //     final int prevLogIndex,
          //     final int prevLogTerm,
          //     final Entry[] entries,
          //     final int leaderCommit) 

        // CandidateMode cm = new CandidateMode();
        // cm.initializeServer(mConfig,mLog,mLastApplied,mRmiPort,mID);
        // RaftServerImpl.setMode(cm);